#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "stack.h"

#define STACK_SIZE 1024

static Element stack[STACK_SIZE];
static int sp = 0; /* stack pointer */


int try_stack_peek(Element *out_el) {
    if (sp > 0) {
        if(out_el) {
            *out_el = stack[sp - 1];
        }
        return 1;
    };
    return 0;
}

void stack_pop() {
    if(sp > 0) {
        sp--;
    }
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


/* unit tests */

static void assert_stack_empty() {
    
    int empty = !try_stack_peek(NULL);
    assert(empty);
}

static void assert_stack_pop(const Element *expect) {
    Element el = {0}; 
    
    int exists = try_stack_peek(&el);
    stack_pop();

    int eq = element_equals(expect, &el);
    assert(exists && eq);
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

