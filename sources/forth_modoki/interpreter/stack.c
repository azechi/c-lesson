#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "clesson.h"

#define STACK_SIZE 1024

static Element stack[STACK_SIZE];
static int sp = 0; /* stack pointer */


Element *try_stack_pop() {
    if(sp > 0) {
        return &stack[--sp];
    }

    return NULL;
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
    Element input = {0};
    Element expect = {0};

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


__attribute__((unused))
static void test_all() {
    test_try_stack_pop();
    test_stack_push_pop();
    test_stack_push_push_pop_pop();

    Element el = {ELEMENT_NUMBER, .u.number = 123};
    stack_push(&el);

    el = (Element){ELEMENT_NUMBER, .u.number = 45};
    stack_push(&el);

    el = (Element){ELEMENT_LITERAL_NAME, .u.name = "some"};
    stack_push(&el);

    el = (Element){ELEMENT_LITERAL_NAME, .u.name = "some2"};
    stack_push(&el);

    stack_print_all();
}

#if 0
int main() {
    test_all();

    return 0;
}
#endif
