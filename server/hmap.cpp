#include <unordered_map>
#include <cstdlib>
#include <cstring>

#include "include/hmap.h"

using c_map = std::unordered_map<int, char*>;

clients_map clients_map_new() 
{
    return new c_map();
}

char* clients_map_get(clients_map map, int client_sfd) 
{
    c_map* m = static_cast<c_map*>(map);
    
    auto it = m->find(client_sfd);
    if (it != m->end()) {
        return it->second;
    }
    
    return NULL;
}

bool clients_map_has(clients_map map, int client_sfd)
{
    c_map* m = static_cast<c_map*>(map);
    
    auto it = m->find(client_sfd);
    if (it != m->end()) {
        return true;
    }
    
    return false;
}

void clients_map_set(clients_map map, int client_sfd, char* room_id) 
{
    c_map* m = static_cast<c_map*>(map);
    m->insert(
        {client_sfd, strdup(room_id)}
    );
}

void clients_map_free(clients_map map) 
{
    c_map* m = static_cast<c_map*>(map);
    for (auto &p : *m) { 
        free(p.second); 
    }
    delete m;
}