#include <stdlib.h>
#include "assembler_private.h"
#include "assembler_unresolve_address_list.h"


typedef struct Node_ {
    UnresolveAddress item;
    struct Node_ *next;
} Node;


static Node *head = NULL;

void unresolve_address_clear() {
    head = NULL;
}

void unresolve_address_push(UnresolveAddress *ua) {
    Node *n = head;
    while(n) {
        n = n->next;
    }

    n = malloc(sizeof(Node));
    n->item = *ua;
    n->next = head;
    head = n;
}

int unresolve_address_pop(UnresolveAddress *out_ua) {
    if(!head) {
        return 0;
    }

    *out_ua = head->item;

    Node *tmp = head;
    head = head->next;
    free(tmp);;
    return 1;
}

