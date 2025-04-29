#include "lru.h"

lru_mac_entry_t *lru_mac_table = NULL;
lru_node_t *lru_head = NULL, *lru_tail = NULL;

void lru_move_to_front(lru_mac_entry_t *entry) {
    if ((lru_node_t *)entry == lru_head) return;
    if (entry->lru.prev) entry->lru.prev->next = entry->lru.next;
    if (entry->lru.next) entry->lru.next->prev = entry->lru.prev;
    if ((lru_node_t *)entry == lru_tail) lru_tail = entry->lru.prev;
    entry->lru.next = lru_head;
    entry->lru.prev = NULL;
    if (lru_head) lru_head->prev = (lru_node_t *)entry;
    lru_head = (lru_node_t *)entry;
    if (!lru_tail) lru_tail = lru_head;
}

void lru_insert(lru_mac_entry_t *entry) {
    entry->lru.prev = NULL;
    entry->lru.next = lru_head;
    if (lru_head) lru_head->prev = (lru_node_t *)entry;
    lru_head = (lru_node_t *)entry;
    if (!lru_tail) lru_tail = lru_head;
}

void lru_remove_tail() {
    if (!lru_tail) return;
    lru_mac_entry_t *entry = (lru_mac_entry_t *)lru_tail;
    HASH_DEL(lru_mac_table, entry);
    if (entry->lru.prev) entry->lru.prev->next = NULL;
    lru_tail = entry->lru.prev;
    if ((lru_node_t *)entry == lru_head) lru_head = NULL;
    free(entry);
}

void lru_hash_add(uint8_t *mac, int port) {
    lru_mac_entry_t *e;
    HASH_FIND(hh, lru_mac_table, mac, 6, e);
    if (!e) {
        if (HASH_COUNT(lru_mac_table) >= LRU_MAX_ENTRIES) {
            lru_remove_tail();
        }
        e = malloc(sizeof(lru_mac_entry_t));
        memcpy(e->mac, mac, 6);
        e->out_port = port;
        // e->last_seen = time(NULL);
        lru_insert(e);
        HASH_ADD(hh, lru_mac_table, mac, 6, e);
    } else {
        e->out_port = port;
        // e->last_seen = time(NULL);
        lru_move_to_front(e);
    }
}

int lru_hash_lookup(uint8_t *mac) {
    lru_mac_entry_t *e;
    HASH_FIND(hh, lru_mac_table, mac, 6, e);
    if (e) {
        // e->last_seen = time(NULL);
        lru_move_to_front(e);
        return e->out_port;
    }
    return -1;
}
