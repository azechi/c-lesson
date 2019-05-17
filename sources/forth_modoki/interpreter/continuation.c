#include <assert.h>
#include <stdio.h>
#include "continuation.h"


#define STACK_SIZE 1024

static Continuation stack[STACK_SIZE];
static int sp = 0; /* stack pointer */

Continuation *try_co_stack_pop() {
    if(sp > 0) {
        return &stack[--sp];
    }
    return NULL;
}

void co_stack_push(const Continuation *co) {
    assert(sp < STACK_SIZE);

    if(sp < STACK_SIZE) {
        stack[sp++] = *co;
    }
}

void co_stack_push_exec_array(const ElementArray *ea) {
    Continuation co = {.exec_array = ea, .pc = 0};
    co_stack_push(&co);
}


void co_stack_clear() {
    sp = 0;
}

void co_stack_print_all() {
    int i = sp;
    Continuation *co;
    while(i) {
        co = &stack[--i];
        printf("pc: %d\n", co->pc);
        element_array_print(co->exec_array);
    }
}

void co_stack_test_all() {
}


