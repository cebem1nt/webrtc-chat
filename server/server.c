#include <stdbool.h>
#include <stddef.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "include/frames.h"
#include "include/hmap.h"
#include "include/http_p.h"
#include "include/crypt.h"

#define PORT 8080
#define LISTEN_BACKLOG 10
#define MAX_MSG_SIZE 2048

/* 
 * manpages: socket(2), sockaddr_in, listen(2),
 *           ip(7), bind(2), accept(2), htonl
 *
 */


void 
err_exit(const char* reason) 
{
    perror(reason);
    exit(EXIT_FAILURE);
}

/*
 * Gets raw http request, returns SWITCHING_PROTOCOL  
 * respones if request is fine, otherwise null
 */
char*
handshake(char* request_raw)
{
    // 1 - Parse request & Extract key 
    struct http_request req = http_parse_request(request_raw);
    const char* req_key = http_get_header_value(req, "Sec-WebSocket-Key");

    if (strcmp(req.method, "GET") != 0 || req_key == NULL) {
        http_destroy_request(req);
        return NULL; // Invalid request. We aint handle normal http. 
    }

    const char* signed_key = sign_key(req_key);
    struct http_response res = http_new_response(SWITCHING_PROTOCOL);

    http_response_append_header(&res, "Upgrade", "websocket");
    http_response_append_header(&res, "Connection", "Upgrade");
    http_response_append_header(&res, "Sec-WebSocket-Accept", signed_key);

    char* out = http_compose_response(res);
    
    http_destroy_request(req);
    http_destroy_response(res);
    return out;
}

void 
hadle_client(int client_sfd, clients_map cmap) 
{
    char buf[MAX_MSG_SIZE];
    size_t n;

    while ((n = read(client_sfd, buf, MAX_MSG_SIZE)) > 0 ) {
        printf("New message, size: %zd\n", n);

        bool is_recognized = clients_map_has(cmap, client_sfd);

        /* 
         * If we dont have mapped socket fd, then assume incoming request
         * is webrtc handshake. If its not, but we already handshaked, 
         * treat the buffer as data frame.
         */
        if (!is_recognized) {
            printf("Client %i is not recognized, trying to handshake...\n", client_sfd);

            char* response_raw = handshake(buf);
            if (!response_raw)
                break;

            write(client_sfd, response_raw, strlen(response_raw));
            clients_map_set(cmap, client_sfd, NULL); // <- here 
            free(response_raw);
        }

        else if (is_recognized) {
            struct ws_in_frame fr;
            if (ws_parse_frame((unsigned char*)buf, n, &fr)) {
                printf("Parsing error!\n");
            } else {
                printf("Unmasked message: \n%s\n", fr.payload);
                free(fr.payload);
            }
            const char* msg = "Hello client!";
            struct ws_out_frame out;

            if (ws_to_frame((unsigned char*)msg, 13, &out)) {
                printf("Could not frame message\n");
            } else {
                write(client_sfd, out.frame, out.frame_len);
                free(out.frame);
            }
        }
    }

    printf("Bye client!\n");
    close(client_sfd);
}

int 
main()
{
    int server_sfd, client_sfd;
    clients_map cmap = clients_map_new();

    struct sockaddr_in server_addr = {0};
    struct sockaddr    client_addr = {0};

    server_sfd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_sfd == -1)
        err_exit("socket");

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(PORT); 

    int opt = 1;
    setsockopt(server_sfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    if (bind(server_sfd, (struct sockaddr*) &server_addr, sizeof(server_addr)) == -1)
        err_exit("bind");

    if (listen(server_sfd, LISTEN_BACKLOG) == -1)
        err_exit("listen");

    // Listen for any connections
    printf("Server is listening\n");
    while (1) { 
        socklen_t client_addr_s = sizeof(client_addr);
        client_sfd = accept(server_sfd, &client_addr, &client_addr_s);
        
        pid_t pid = fork();
        if (pid == -1)
            err_exit("fork");

        if (pid == 0) {            
            if (client_sfd == -1)
                err_exit("accept");
            
            printf("Client accepted\n");

            close(server_sfd);
            hadle_client(client_sfd, cmap);
            _exit(EXIT_SUCCESS); 
        }
    }

    return 0;
}