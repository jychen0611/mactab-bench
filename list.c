#include "list.h"

list_node_t *list_head = NULL;

void list_add(uint8_t *mac, int port) {
    list_node_t *n = malloc(sizeof(list_node_t));
    memcpy(n->entry.mac, mac, 6);
    n->entry.out_port = port;
    n->next = list_head;
    list_head = n;
}

int list_lookup(uint8_t *mac) {
    list_node_t *cur = list_head;
    while (cur) {
        if (memcmp(cur->entry.mac, mac, 6) == 0)
            return cur->entry.out_port;
        cur = cur->next;
    }
    return -1;
}
