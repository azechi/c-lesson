#include <assert.h>
#include <stdio.h>
#include "continuation.h"


#define STACK_SIZE 1024


static CallStackItem stack[STACK_SIZE];
static int sp = 0; /* stack pointer */

CallStackItem *try_co_stack_peek() {
    if(sp > 0) {
        return &stack[(sp - 1)];
    }
    return NULL;
}

CallStackItem *try_co_stack_pop() {
    if(sp > 0) {
        return &stack[--sp];
    }
    return NULL;
}

void co_stack_push(const CallStackItem *item) {
    assert(sp < STACK_SIZE);

    if(sp < STACK_SIZE) {
        stack[sp++] = *item;
    }
}

void co_stack_push_continuation(const Continuation *co) {
    CallStackItem item = {
        .ctype = CALLSTACKITEM_CONTINUATION,
        .u.continuation = *co
    };
    co_stack_push(&item);
}

void co_stack_push_exec_array(const ElementArray *exec_array) {
    CallStackItem item = {
        .ctype = CALLSTACKITEM_CONTINUATION,
        .u.continuation = {
            .pc = 0,
            .exec_array = exec_array
        }
    };
    co_stack_push(&item);
}

void co_stack_push_variable(const Element *el) {
    CallStackItem item = {
        .ctype = CALLSTACKITEM_VARIABLE,
        .u.variable = *el
    };
    co_stack_push(&item);
}

void co_stack_clear() {
    sp = 0;
}

void co_stack_print_all() {
    int i = sp;
    CallStackItem *item;
    while(i) {
        item = &stack[--i];
        switch(item->ctype) {
            case CALLSTACKITEM_CONTINUATION:
                printf("pc: %d\n", item->u.continuation.pc);
                element_array_print(item->u.continuation.exec_array);
                break;
            case CALLSTACKITEM_VARIABLE:
                printf("variable: ");
                element_print(&item->u.variable);
        }
    }
}

void co_stack_test_all() {
}


