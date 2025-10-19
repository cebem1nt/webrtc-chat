/*
 * For our webrtc server we need a simple hashmap wrapper for
 * an array of clients connected to it, as well, that would be nice to have
 * a hashset for connected clients.
 */

#ifndef _HMAP_H
#define _HMAP_H

#define MAX_CLIENTS_PER_ROOM 10

typedef void* rooms_hmap;
typedef void* clients_hset;

struct client {
    char* room_id;
};

rooms_hmap rooms_hmap_new();

int* rooms_hmap_get(rooms_hmap map, char* room_id);

bool rooms_hmap_has(rooms_hmap map, char* room_id);

void rooms_hmap_append_client(rooms_hmap map, char* room_id, int client_id);

void rooms_map_delete_client(rooms_hmap map, char* room_id, int client_id);

void rooms_hmap_free(rooms_hmap map);


rooms_hmap clients_hset_new();

int clients_hset_get(clients_hset hset, int id);

bool clients_hset_has(clients_hset hset, int id);

void clients_hset_set(clients_hset hset, int id);

void clients_hset_delete(clients_hset hset, int id);

void clients_hset_free(clients_hset hset);

#endif