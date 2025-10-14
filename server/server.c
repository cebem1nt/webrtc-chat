#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "http_p.h"

#define PORT 8000
#define LISTEN_BACKLOG 10
#define MSG_BUFFER_SIZE 2048

/* 
 * manpages: socket(2), sockaddr_in, listen(2),
 *           ip(7), bind(2), accept(2), htonl
 *
 * TODO We need asynchrony
 */

void err_exit(char* reason) 
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
    printf("\nParsed line: %u, %s\n\n", req.method, req.path);
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

    if (bind(server_sfd, (struct sockaddr*) &server_addr, sizeof(server_addr)) == -1)
        err_exit("bind");

    if (listen(server_sfd, LISTEN_BACKLOG) == -1)
        err_exit("listen");

    // Listen for any connections
    while (1) { 
        socklen_t client_addr_s = sizeof(client_addr);
        client_sfd = accept(server_sfd, &client_addr, &client_addr_s);

        if (client_sfd == -1)
            err_exit("accept");

        hadle_client(client_sfd);
    }

    return 0;
}