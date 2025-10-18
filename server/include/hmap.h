/*
 * For our webrtc server we need a simple hashmap wrapper that
 * will be able to store aditional info about client based on it's fd
 */

#ifndef _HMAP_H
#define _HMAP_H

typedef void* clients_map;

struct client {
    char* room_id;
};

clients_map clients_map_new();

char* clients_map_get(clients_map map, int client_sfd);

bool clients_map_has(clients_map map, int client_sfd);

void clients_map_set(clients_map map, int client_sfd, char* room_id);

void clients_map_free(clients_map map);

#endif