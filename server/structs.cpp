#include <cstddef>
#include <unordered_map>
#include <cstdlib>
#include <cstring>
#include <unordered_set>

#include "include/structs.h"

using rhmap = std::unordered_map<char*, int*>; // Array of integers
using chset = std::unordered_set<int>;

rooms_hmap rooms_hmap_new() 
{
    return new rhmap();
}

int* rooms_hmap_get(rooms_hmap map, char* room_id)
{
    rhmap* m = static_cast<rhmap*>(map);
    
    auto it = m->find(room_id);
    if (it != m->end()) {
        return it->second;
    }
    
    return NULL;
}

bool rooms_hmap_has(rooms_hmap map, char* room_id)
{
    rhmap* m = static_cast<rhmap*>(map);
    
    auto it = m->find(room_id);
    if (it != m->end()) {
        return true;
    }
    
    return false;
}

void rooms_hmap_append_client(rooms_hmap map, char* room_id, int client_id)
{
    int* clients_in_room = rooms_hmap_get(map, room_id); 
    rhmap* m = static_cast<rhmap*>(map);

    if (clients_in_room) {
        for (int i = 0; i < MAX_CLIENTS_PER_ROOM; i++) {
            if (clients_in_room[i] == -1)
                clients_in_room[i] = client_id;
        }
    } 
    else {
        clients_in_room = new int[MAX_CLIENTS_PER_ROOM];
        clients_in_room[0] = client_id;
    }

    m->insert(
        {strdup(room_id), clients_in_room}
    );
}

void rooms_map_delete_client(rooms_hmap map, char* room_id, int client_id)
{
    int* clients_in_room = rooms_hmap_get(map, room_id); 
    rhmap* m = static_cast<rhmap*>(map);

    if (clients_in_room) {
        for (int i = 0; i < MAX_CLIENTS_PER_ROOM; i++) {
            if (clients_in_room[i] == client_id)
                clients_in_room[i] = -1;
        }
    }
   
    m->insert(
        {strdup(room_id), clients_in_room}
    );
}

void rooms_hmap_free(rooms_hmap map)
{
    rhmap* m = static_cast<rhmap*>(map);
    for (auto &p : *m) { 
        free(p.first); 
        free(p.second); 
    }
    delete m;
}

rooms_hmap clients_hset_new()
{
    return new chset();
}

int clients_hset_get(clients_hset hset, int id)
{
    chset* s = static_cast<chset*>(hset);
    
    auto it = s->find(id);
    if (it != s->end()) {
        return *it;
    }
    
    return -1;
}

bool clients_hset_has(clients_hset hset, int id)
{
    return clients_hset_get(hset, id) != -1;
}

void clients_hset_set(clients_hset hset, int id)
{
    chset* s = static_cast<chset*>(hset);
    s->insert(id);
}

void clients_hset_delete(clients_hset hset, int id)
{
    chset* s = static_cast<chset*>(hset);
    s->erase(id);
}

void clients_hset_free(clients_hset hset)
{
    chset* s = static_cast<chset*>(hset);
    delete s;
}