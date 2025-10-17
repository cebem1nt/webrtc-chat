#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "include/frames.h"
#include "include/http_p.h"
#include "include/crypt.h"

#define PORT 8080
#define LISTEN_BACKLOG 10
#define MAX_MSG_SIZE 2048

/* 
 * manpages: socket(2), sockaddr_in, listen(2),
 *           ip(7), bind(2), accept(2), htonl
 *
 * TODO We need asynchrony
 */

void 
err_exit(const char* reason) 
{
    perror(reason);
    exit(EXIT_FAILURE);
}

const char*
handle_handshake(const char* req_key)
{
    char* signed_key = sign_key(req_key);

    struct http_response res = http_new_response(SWITCHING_PROTOCOL);

    http_response_append_header(&res, "Upgrade", "websocket");
    http_response_append_header(&res, "Connection", "Upgrade");
    http_response_append_header(&res, "Sec-WebSocket-Accept", signed_key);

    const char* out = http_compose_response(res);
    http_destroy_response(res);
    return out;
}

void 
hadle_client(int client_sfd) 
{
    char buf[MAX_MSG_SIZE];
    size_t n;

    while ( (n = read(client_sfd, buf, MAX_MSG_SIZE)) > 0 ) {
        printf("Request size: %zd\n", n);

        struct http_request req = http_parse_request(buf);
        const char* req_key = http_get_header_value(req, "Sec-WebSocket-Key");

        if (strcmp(req.method, "GET") == 0 && req_key) {
            const char* res = handle_handshake(req_key);
            printf("Composed response: \n%s\n", res);
            write(client_sfd, res, strlen(res));
        } 
        else {
            struct ws_in_frame fr;
            if (ws_parse_frame((unsigned char*)buf, n, &fr)) {
                printf("Parsing error\n");
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

        http_destroy_request(req);
    }

    printf("Bye client!\n");
    close(client_sfd);
}

int 
main()
{
    int server_sfd;
    int client_sfd;
    struct sockaddr_in  server_addr = {0};
    struct sockaddr     client_addr = {0};

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

        if (client_sfd == -1)
            err_exit("accept");
        
        printf("Client accepted\n");
        hadle_client(client_sfd);
    }

    return 0;
}