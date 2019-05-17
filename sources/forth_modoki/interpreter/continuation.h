#ifndef CO_STACK_H
#define CO_STACK_H

#include "element.h"

typedef struct Continuation_ {
    const ElementArray *exec_array;
    int pc; /* program counter  */
} Continuation;


Continuation *try_co_stack_pop();

void co_stack_push(const Continuation *co);

void co_stack_push_exec_array(const ElementArray *ea);

void co_stack_clear();
void co_stack_print_all();
void co_stack_test_all();


#endif
