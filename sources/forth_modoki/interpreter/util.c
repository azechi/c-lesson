#include <string.h>
#include <stdio.h>
#include "def.h"
#include "util.h"



int streq(const char *s1, const char *s2) {
    return (strcmp(s1, s2) == 0);
}


void print_indent(int i) {
    char str[5]; /* "%99c"" */
    sprintf(str, "%%%dc", i % 100);
    printf(str, ' ');
}

/* stack */

int try_stack_pop_element(Element *out_el) {
    int exists = try_stack_peek(out_el);
    if(exists) {
        stack_pop();
    }
    return exists;
}

void stack_pop_element(Element *out_el) {
    if(!try_stack_pop_element(out_el)){
        assert_fail("STACKUNDERFLOW");
    }
}

int stack_pop_number() {
    Element el = {0};
    stack_pop_element(&el);

    if(el.etype != ELEMENT_NUMBER) {
        assert_fail("NOT NUMBER ELEMENT");
    }
    return el.u.number;
}

char *stack_pop_literal_name() {
    Element el = {0};
    stack_pop_element(&el);

    if(el.etype != ELEMENT_LITERAL_NAME) {
        assert_fail("NOT LITRAL NAME ELEMENT");
    }
    return el.u.name;
}

ElementArray *stack_pop_exec_array() {
    Element el = {0};
    stack_pop_element(&el);

    if(el.etype != ELEMENT_EXEC_ARRAY) {
        assert_fail("NOT EXEC ARRAY ELEMENT");
    }
    return el.u.exec_array;
}

void stack_push_number(int number) {
    Element el = {ELEMENT_NUMBER, .u.number = number};
    stack_push(&el);
}

void stack_push_excutable_name(char *name) {
    Element el = {ELEMENT_EXECUTABLE_NAME, .u.name = name};
    stack_push(&el);
}

void stack_push_exec_array(ElementArray *exec_array) {
    Element el = {ELEMENT_EXEC_ARRAY, .u.exec_array = exec_array};
    stack_push(&el);
}

void stack_print() {
    Element el = {0};
    if(!try_stack_pop_element(&el)) {
        return;
    }

    element_print(&el);
    stack_print();
    stack_push(&el);
}
