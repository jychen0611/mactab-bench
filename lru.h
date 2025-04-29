#ifndef LRU_H
#define LRU_H

#include <stdint.h>
#include "ut_hash.h"

typedef struct lru_node {
    struct lru_node *prev;
    struct lru_node *next;
} lru_node_t;

typedef struct lru_mac_entry {
    uint8_t mac[6];
    int out_port;
    UT_hash_handle hh; // <-- must be BEFORE lru_node!
    lru_node_t lru;
} lru_mac_entry_t;

void lru_hash_add(uint8_t *mac, int port);
int lru_hash_lookup(uint8_t *mac);
void lru_free_all(void);

#endif // LRU_H

