#include <stdint.h>
#include <string.h>
#include "ut_hash.h"

/* uthash Implementation */
typedef struct {
    uint8_t mac[6];
    int out_port;
    UT_hash_handle hh;
} mac_entry_t;

void hash_add(uint8_t *mac, int port);

int hash_lookup(uint8_t *mac);