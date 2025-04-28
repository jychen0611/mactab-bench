#include <stdint.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    uint8_t mac[6];
    int out_port;
} fwd_entry_t;

/* Linked List Implementation */
typedef struct list_node {
    fwd_entry_t entry;
    struct list_node *next;
} list_node_t;

void list_add(uint8_t *mac, int port);

int list_lookup(uint8_t *mac);