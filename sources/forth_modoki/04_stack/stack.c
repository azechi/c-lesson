#include <stdio.h>
#include <assert.h>
#include <string.h>
#include "stack.h"

#define STACK_SIZE 1024

static struct Token stack[STACK_SIZE];
static int sp = 0; // stack pointer

struct Token *stack_pop() {
    if(sp > 0) {
        return &stack[--sp];
    }

    return NULL;
}

void stack_push(const struct Token *token) {
    assert(sp < STACK_SIZE);

    if(sp < STACK_SIZE) {
        stack[sp++] = *token;
    }
}

static void stack_clear() {
    sp = 0;
}

static void stack_print_all() {
    struct Token *t;
    while((t = stack_pop())) {
        switch(t->ltype) {
            case NUMBER:
                printf("num: %d\n", t->u.number);
                break;
            case LITERAL_NAME:
                printf("LITERAL_NAME: %s\n", t->u.name);
                break;
            default:
                printf("Unknown type %d\n", t->ltype);
                break;
        }
    }
}

static int token_equals(const struct Token t1, const struct Token t2) {
    if(t1.ltype == t2.ltype) {
        switch(t1.ltype) {
            case NUMBER:
                return (t1.u.number == t2.u.number);
            case LITERAL_NAME:
                return (strcmp(t1.u.name, t2.u.name) == 0);
            default:
                return 0;
        }
    }
    return 0;
}

/* unit tests */

static void test_token_equals() {
    struct Token input_number_0 = {NUMBER, {0}};
    struct Token input_number_0_0 = {NUMBER, {0}};
    struct Token input_number_1 = {NUMBER, {1}};
    struct Token input_literal_name_0 = {LITERAL_NAME, .u.name = "a"};
    struct Token input_literal_name_0_0 = {LITERAL_NAME, .u.name = "a"};
    struct Token input_literal_name_1 = {LITERAL_NAME, .u.name = "b"};

    assert(token_equals(input_number_0, input_number_0_0));
    assert(!token_equals(input_number_0, input_number_1));

    assert(token_equals(input_literal_name_0, input_literal_name_0_0));
    assert(!token_equals(input_literal_name_0, input_literal_name_1));
}

static void test_stack_pop() {
    struct Token *actual = stack_pop();

    assert(actual == NULL);
}

static void test_stack_push() {
    struct Token input = {0};

    stack_push(&input);

    assert(sp == 1);
    assert(token_equals(stack[0], input));
}

static void test_stack_push_pop() {
    struct Token input = {0};

    struct Token actual;

    stack_push(&input);
    actual = *stack_pop();

    assert(sp == 0);
    assert(token_equals(actual, input));
}

static void test_stack_push_push_pop_pop() {
    struct Token input_1 = {NUMBER, {0}};
    struct Token input_2 = {NUMBER, {1}};

    struct Token actual;

    stack_push(&input_1);
    stack_push(&input_2);

    actual = *stack_pop();
    assert(token_equals(actual, input_2));

    actual = *stack_pop();
    assert(token_equals(actual, input_1));
}


int main() {

    test_token_equals();

    stack_clear();
    test_stack_pop();

    stack_clear();
    test_stack_push();

    stack_clear();
    test_stack_push_pop();

    stack_clear();
    test_stack_push_push_pop_pop();


    stack_push(&(struct Token){NUMBER, .u.number = 123});
    stack_push(&(struct Token){NUMBER, .u.number = 45});
    stack_push(&(struct Token){LITERAL_NAME, .u.name = "some"});
    stack_push(&(struct Token){LITERAL_NAME, .u.name = "some2"});

    stack_print_all();

    return 1;
}
