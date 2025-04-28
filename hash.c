#include "hash.h"

mac_entry_t *mac_table = NULL;

void hash_add(uint8_t *mac, int port) {
    mac_entry_t *e;
    HASH_FIND(hh, mac_table, mac, 6, e);
    if (!e) {
        e = malloc(sizeof(mac_entry_t));
        memcpy(e->mac, mac, 6);
        e->out_port = port;
        HASH_ADD(hh, mac_table, mac, 6, e);
    } else {
        /* Update port */
        e->out_port = port;
    }
}

int hash_lookup(uint8_t *mac) {
    mac_entry_t *e;
    HASH_FIND(hh, mac_table, mac, 6, e);
    return e ? e->out_port : -1;
}