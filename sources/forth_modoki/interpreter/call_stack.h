#ifndef CALL_STACK_H
#define CALL_STACK_H


#include "element.h"


typedef struct Continuation_ {
    const ElementArray *exec_array;
    int pc; /* program counter */
} Continuation;

typedef void (*Execute)(Continuation*);


void eval_exec_array(Execute execute, const ElementArray *exec_array);

void call_exec_array(const ElementArray *exec_array, const Continuation *continuation);


void local_variable_get(int offset, Element *out_el);

void local_variable_pop();

void local_variable_push(const Element *el);


#endif
