#include <asm-generic/socket.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "http_p.h"

#define PORT 8080
#define LISTEN_BACKLOG 10
#define MSG_BUFFER_SIZE 2048

/* 
 * manpages: socket(2), sockaddr_in, listen(2),
 *           ip(7), bind(2), accept(2), htonl
 *
 * TODO We need asynchrony
 */

void err_exit(const char* reason) 
{
    perror(reason);
    exit(EXIT_FAILURE);
}

void hadle_client(int client_sfd) 
{
    // Basicaly for now we have to handle the
    // Websocket handshake using http requests
    char buf[MSG_BUFFER_SIZE];
    read(client_sfd, buf, MSG_BUFFER_SIZE);

    struct http_request req = parse_http_request(buf);
    printf("Lines: %zu\n", req.headers_length);

    for (int i = 0; i < req.headers_length; i++) {
        printf("Header - %s: %s\n", req.headers[i].name, req.headers[i].data);
    }
}

int main()
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