#ifndef CLESSON_H
#define CLESSON_H


#include <stdio.h>
#include <string.h>

#include "element.h"
#include "stack.h"



int streq(const char *s1, const char *s2);

void print_indent(int i);


int try_stack_pop_element(Element *out_el);

void stack_pop_element(Element *out_el);

int stack_pop_number();

char *stack_pop_literal_name();

ElementArray *stack_pop_exec_array();



void stack_push_number(int i);

void stack_push_exec_array(ElementArray *ea);

void stack_push_executable_name(char *name);

void stack_print();


#endif
