#include <stdint.h> 
#include "ut_hash.h"

#define LRU_MAX_ENTRIES 100000

typedef struct lru_node {
    struct lru_node *prev;
    struct lru_node *next;
} lru_node_t;

typedef struct lru_mac_entry {
    uint8_t mac[6];
    int out_port;
    lru_node_t lru;
    UT_hash_handle hh;
} lru_mac_entry_t;

void lru_move_to_front(lru_mac_entry_t *entry);

void lru_insert(lru_mac_entry_t *entry);

void lru_remove_tail();

void aging_cleanup(); 

void lru_hash_add(uint8_t *mac, int port);

int lru_hash_lookup(uint8_t *mac);

