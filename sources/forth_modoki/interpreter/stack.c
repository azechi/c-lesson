#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "util.h"
#include "stack.h"

#define STACK_SIZE 1024

static Element stack[STACK_SIZE];
static int sp = 0; /* stack pointer */


Element *try_stack_pop() {
    if(sp > 0) {
        return &stack[--sp];
    }

    return NULL;
}

Element *stack_pop() {
    Element *el = try_stack_pop();
    if(!el){
        assert_fail("STACKUNDERFLOW");
    }
    return el;
}

int stack_pop_number() {
    Element *el = stack_pop();

    if(ELEMENT_NUMBER != el->etype){
        assert_fail("NOT NUMBER ELEMENT");
    }

    return el->u.number;
}

char *stack_pop_literal_name() {
    Element *el = stack_pop();

    if(ELEMENT_LITERAL_NAME != el->etype) {
        assert_fail("NOT LITERAL_NAME");
    }

    return el->u.name;
}

ElementArray *stack_pop_exec_array() {
    Element *el = stack_pop();

    if(ELEMENT_EXEC_ARRAY != el->etype) {
        assert_fail("NOT EXEC ARRAY");
    }

    return el->u.exec_array;
}

void stack_push_number(int i) {
    Element el = {ELEMENT_NUMBER, .u.number = i};
    stack_push(&el);
}

void stack_push_exec_array(ElementArray *ea) {
    Element el = {ELEMENT_EXEC_ARRAY, .u.exec_array = ea};
    stack_push(&el);
}

void stack_push(const Element *el) {
    assert(sp < STACK_SIZE);

    if(sp < STACK_SIZE) {
        stack[sp++] = *el;
    }
}


void stack_clear() {
    sp = 0;
}


void stack_print_all() { 
    int i = sp;
    while(i > 0) {
        element_print(&stack[--i]);
    };
}


/* unit tests */

static void assert_stack_empty() {
    Element *actual = try_stack_pop();
    assert(!actual);
}

static void assert_stack_pop(const Element *expect) {
    Element *el = try_stack_pop();

    int actual = element_equals(expect, el);
    assert(actual);
}


static void test_try_stack_pop() {
    assert_stack_empty();
}

static void test_stack_push_pop() {
    Element input = {ELEMENT_NUMBER, {0}};
    Element expect = {ELEMENT_NUMBER, {0}};

    stack_push(&input);

    assert_stack_pop(&expect);
    assert_stack_empty();
}

static void test_stack_push_push_pop_pop() {
    Element input_1 = {ELEMENT_NUMBER, {0}};
    Element input_2 = {ELEMENT_NUMBER, {1}};

    Element expect_1 = {ELEMENT_NUMBER, {1}};
    Element expect_2 = {ELEMENT_NUMBER, {0}};

    stack_push(&input_1);
    stack_push(&input_2);

    assert_stack_pop(&expect_1);
    assert_stack_pop(&expect_2);
    assert_stack_empty();
}


void stack_test_all() {
    test_try_stack_pop();
    test_stack_push_pop();
    test_stack_push_push_pop_pop();
}

#if 0
int main() {
    test_all();

    return 0;
}
#endif
