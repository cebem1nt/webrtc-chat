#include <cstddef>
#include <cstdio>
#include <unordered_map>
#include <cstdlib>
#include <cstring>
#include <unordered_set>
#include <string>

#include "include/structs.h"

using rhmap = std::unordered_map<std::string, int*>;
using chset = std::unordered_set<int>;

rooms_hmap rooms_hmap_new()
{
    return new rhmap();
}

int* rooms_hmap_get(rooms_hmap map, char* room_id)
{
    if (!map || !room_id) 
        return NULL;

    rhmap* m = static_cast<rhmap*>(map);
    auto it = m->find(std::string(room_id));

    if (it == m->end()) 
        return NULL;

    int* src = it->second;
    if (!src) 
        return NULL;

    int* copy = (int*) malloc(sizeof(int) * MAX_CLIENTS_PER_ROOM);
    memcpy(copy, src, sizeof(int) * MAX_CLIENTS_PER_ROOM);

    return copy;
}

bool rooms_hmap_has(rooms_hmap map, char* room_id)
{
    if (!map || !room_id) 
        return false;

    rhmap* m = static_cast<rhmap*>(map);
    return m->find(std::string(room_id)) != m->end();
}

void rooms_hmap_append_client(rooms_hmap map, char* room_id, int client_id)
{
    if (!map || !room_id) 
        return;

    rhmap* m = static_cast<rhmap*>(map);
    std::string key(room_id);

    int* arr = NULL;
    auto it = m->find(key);
    
    if (it == m->end()) {
        arr = (int*) malloc(sizeof(int) * MAX_CLIENTS_PER_ROOM);
        for (int i = 0; i < MAX_CLIENTS_PER_ROOM; ++i) {
            arr[i] = -1;
        }
        arr[0] = client_id;
        m->insert({key, arr});
        return;
    }
    else {
        arr = it->second;
    }

    for (int i = 0; i < MAX_CLIENTS_PER_ROOM; ++i) {
        if (arr[i] == client_id) 
            return;
        if (arr[i] == -1) { 
            arr[i] = client_id; 
            return; 
        }
    }
}

void rooms_map_delete_client(rooms_hmap map, char* room_id, int client_id)
{
    if (!map || !room_id) 
        return;

    rhmap* m = static_cast<rhmap*>(map);

    auto it = m->find(std::string(room_id));
    if (it == m->end()) 
        return;

    int* arr = it->second;
    if (!arr) 
        return;

    for (int i = 0; i < MAX_CLIENTS_PER_ROOM; ++i) {
        if (arr[i] == client_id) {
            arr[i] = -1;
        }
    }
}

void rooms_hmap_free(rooms_hmap map)
{
    if (!map) 
        return;
    rhmap* m = static_cast<rhmap*>(map);
    for (auto &p : *m) {
        free(p.second);
    }
    delete m;
}

clients_hset clients_hset_new()
{
    return new chset();
}

int clients_hset_get(clients_hset hset, int id)
{
    if (!hset) 
        return -1;

    chset* s = static_cast<chset*>(hset);

    auto it = s->find(id);
    if (it != s->end()) {
        return *it;
    }

    return -1;
}

bool clients_hset_has(clients_hset hset, int id)
{
    if (!hset) 
        return false;
   
    chset* s = static_cast<chset*>(hset);
    return s->find(id) != s->end();
}

void clients_hset_set(clients_hset hset, int id)
{
    if (!hset) 
        return;
    chset* s = static_cast<chset*>(hset);
    s->insert(id);
}

void clients_hset_delete(clients_hset hset, int id)
{
    if (!hset) 
        return;
    chset* s = static_cast<chset*>(hset);
    s->erase(id);
}

void clients_hset_free(clients_hset hset)
{
    if (!hset) 
        return;

    chset* s = static_cast<chset*>(hset);
    delete s;
}