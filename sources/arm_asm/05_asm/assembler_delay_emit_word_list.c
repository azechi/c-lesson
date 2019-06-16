#include <stdlib.h>
#include "assembler_delay_emit_word_list.h"


typedef struct Node_ {
    DelayEmitWord item;
    struct Node_ *next;
} Node;

Node *head = NULL;

void delay_emit_word_clear() {
    head = NULL;
}

void delay_emit_word_push(DelayEmitWord *item) {
    Node **tail = &head;
    while (*tail) {
        tail = &((*tail)->next);
    }

    *tail = malloc(sizeof(Node));
    (*tail)->item = *item;
    (*tail)->next = NULL;
}

int delay_emit_word_pop(DelayEmitWord *out_item) {
    if(!head) {
        return 0;
    }

    *out_item = head->item;

    Node *tmp = head;
    head = head->next;
    free(tmp);
    return 1;
}

