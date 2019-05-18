#ifndef CO_STACK_H
#define CO_STACK_H

#include "element.h"


typedef enum CallStackItemType_ {
    CALLSTACKITEM_VARIABLE,
    CALLSTACKITEM_CONTINUATION
} CallStackItemType;


typedef struct Continuation_ {
    const ElementArray *exec_array;
    int pc; /* program counter  */
} Continuation;


typedef struct CallStackItem_ {
    enum CallStackItemType_ ctype;
    union {
        struct Continuation_ continuation;
        Element variable;
    } u;
} CallStackItem;

CallStackItem  *try_co_stack_peek();

CallStackItem  *try_co_stack_pop();


void co_stack_push(const CallStackItem *item);

void co_stack_push_continuation(const Continuation *co);

void co_stack_push_exec_array(const ElementArray *exec_array);

void co_stack_push_variable(const Element *el);

void co_stack_clear();
void co_stack_print_all();
void co_stack_test_all();


#endif
