#ifndef STACK_H
#define STACK_H

#include "element.h"

/* return NULL if stack empty */
Element *try_stack_pop();

Element *stack_pop();

int stack_pop_number();

char* stack_pop_literal_name();

ElementArray *stack_pop_exec_array();


void stack_push(const Element *el);

void stack_push_number(int i);

void stack_push_exec_array(ElementArray *ea);


void stack_clear();


void stack_print_all();

void stack_test_all();


#endif
