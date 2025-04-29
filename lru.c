#include "lru.h"
#include <stdlib.h>
#include <string.h>

static lru_mac_entry_t *lru_mac_table = NULL;
static lru_node_t *lru_head = NULL, *lru_tail = NULL;

int global_lru_max_entries = 100000;

static void lru_move_to_front(lru_mac_entry_t *entry) {
    if (&entry->lru == lru_head) return;

    if (entry->lru.prev) entry->lru.prev->next = entry->lru.next;
    if (entry->lru.next) entry->lru.next->prev = entry->lru.prev;
    if (&entry->lru == lru_tail) lru_tail = entry->lru.prev;

    entry->lru.prev = NULL;
    entry->lru.next = lru_head;
    if (lru_head) lru_head->prev = &entry->lru;
    lru_head = &entry->lru;
    if (!lru_tail) lru_tail = lru_head;
}

static void lru_insert(lru_mac_entry_t *entry) {
    entry->lru.prev = NULL;
    entry->lru.next = lru_head;
    if (lru_head) lru_head->prev = &entry->lru;
    lru_head = &entry->lru;
    if (!lru_tail) lru_tail = lru_head;
}

static void lru_remove_tail() {
    if (!lru_tail) return;

    // Correct way to find lru_mac_entry_t from lru_node_t
    lru_mac_entry_t *entry = (lru_mac_entry_t *)((char *)lru_tail - offsetof(lru_mac_entry_t, lru));

    HASH_DEL(lru_mac_table, entry);

    if (entry->lru.prev)
        entry->lru.prev->next = NULL;
    lru_tail = entry->lru.prev;

    if (&entry->lru == lru_head)
        lru_head = NULL;

    free(entry);
}

void lru_hash_add(uint8_t *mac, int port) {
    lru_mac_entry_t *e;
    HASH_FIND(hh, lru_mac_table, mac, 6, e);
    if (!e) {
        if (HASH_COUNT(lru_mac_table) >= global_lru_max_entries) {
            lru_remove_tail();
        }
        e = malloc(sizeof(lru_mac_entry_t));
        memcpy(e->mac, mac, 6);
        e->out_port = port;
        lru_insert(e);
        HASH_ADD(hh, lru_mac_table, mac, 6, e);
    } else {
        e->out_port = port;
        lru_move_to_front(e);
    }
}

int lru_hash_lookup(uint8_t *mac) {
    lru_mac_entry_t *e;
    HASH_FIND(hh, lru_mac_table, mac, 6, e);
    if (e) {
        lru_move_to_front(e);
        return e->out_port;
    }
    return -1;
}

void lru_free_all(void) {
    lru_mac_entry_t *e, *tmp;
    HASH_ITER(hh, lru_mac_table, e, tmp) {
        HASH_DEL(lru_mac_table, e);
        free(e);
    }
    lru_head = lru_tail = NULL;
}