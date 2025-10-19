#include <signal.h>
#include <stdbool.h>
#include <stddef.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "include/frames.h"
#include "include/structs.h"
#include "include/structs.h"
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

int
handle_websocket(int client_sfd, char* msg_raw, size_t msg_size)
{
    unsigned char* umsg_raw = (unsigned char*) msg_raw;
    struct ws_in_frame  inf;
    struct ws_out_frame outf;

    if (ws_parse_frame(umsg_raw, msg_size, &inf)) {
        printf("Error when parsing ws frame.\n");
        return 1;
    }

    char* message = (char*) inf.payload;

    switch (inf.opcode) {
    case WSOP_TEXT:
        break;

    default:
    case WSOP_EXIT:
        // Remove client!!!
        return 2;
    }

    if (message[0] != '{') {
        char* room_id = NULL;
        char* p = strchr(message, ':');
        
        if (p && *(p+1) != '\0') {
            room_id = p+1;
            printf("%s", room_id);
            if (ws_to_frame((unsigned char*)room_id, strlen(room_id), &outf)) {
                printf("Could not frame the response.\n");
            } 
        }

        write(client_sfd, outf.payload, outf.frame_len);
        free(outf.payload);
    }

    free(inf.payload);
    return 0;
}

/*
 * Gets raw http request, returns SWITCHING_PROTOCOL  
 * respones if request is fine, otherwise null
 */
char*
handshake(char* request_raw)
{
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
hadle_client(int client_sfd, clients_hset chs) 
{
    char buf[MAX_MSG_SIZE];
    size_t n;

    while ((n = read(client_sfd, buf, MAX_MSG_SIZE)) > 0 ) {
        printf("New message, size: %zd\n", n);

        bool is_recognized = clients_hset_has(chs, client_sfd);

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
            clients_hset_set(chs, client_sfd);
            free(response_raw);
        }

        else if (is_recognized) {
            int code = handle_websocket(client_sfd, buf, n);
            if (code == 2) {
                break;
            }
        }
    }

    printf("Bye client!\n");
    close(client_sfd);
    _exit(EXIT_SUCCESS);
}

int 
main()
{
    int server_sfd, client_sfd;

    clients_hset chs = clients_hset_new();
    // rooms_hmap rmap = rooms_hmap_new();

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
    signal(SIGCHLD, SIG_IGN); // We will ignore return codes of children, i don't like zombie processes

    while (1) { 
        socklen_t client_addr_s = sizeof(client_addr);

        client_sfd = accept(server_sfd, &client_addr, &client_addr_s);
        if (client_sfd == -1)
            err_exit("accept");
        
        pid_t pid = fork();
        if (pid == -1)
            err_exit("fork");

        if (pid == 0) {            
            printf("Client accepted\n");

            close(server_sfd);
            hadle_client(client_sfd, chs);
        }
    }

    return 0;
}