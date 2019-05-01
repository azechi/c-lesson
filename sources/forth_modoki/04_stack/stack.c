#include <stdio.h>
#include "stack.h"

#define STACK_SIZE 1024

static struct Token *stack[STACK_SIZE];
static int sp = 0; // stack pointer

struct Token *stack_pop() {
    if(sp > 0) {
        return stack[--sp];
    }

    return NULL;
}
