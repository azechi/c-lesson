#ifndef STACK_H
#define STACK_H


#include <stddef.h>
#include "element.h"


/*
    out_el parameter can assign NULL
*/
int try_stack_peek(Element *out_el);


void stack_pop();


void stack_push(const Element *el);


void stack_clear();


void stack_test_all();



#endif
