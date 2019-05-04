#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "clesson.h"

#define STACK_SIZE 1024

static struct Element stack[STACK_SIZE];
static int sp = 0; /* stack pointer */

struct Element *stack_pop() {
    if(sp > 0) {
        return &stack[--sp];
    }

    return NULL;
}

void stack_push(const struct Element *el) {
    assert(sp < STACK_SIZE);

    if(sp < STACK_SIZE) {
        stack[sp++] = *el;
    }
}

void stack_clear() {
    sp = 0;
}

static void stack_print_all() {
    struct Element *el;
    while((el = stack_pop())) {
        switch(el->etype) {
            case ELEMENT_NUMBER:
                printf("num: %d\n", el->u.number);
                break;
            case ELEMENT_LITERAL_NAME:
                printf("ELEMENT_LITERAL_NAME: %s\n", el->u.name);
                break;
            default:
                printf("Unknown type %d\n", el->etype);
                break;
        }
    }
}

static int token_equals(const struct Element e1, const struct Element e2) {
    if(e1.etype == e2.etype) {
        switch(e1.etype) {
            case ELEMENT_NUMBER:
                return (e1.u.number == e2.u.number);
            case ELEMENT_LITERAL_NAME:
                return (strcmp(e1.u.name, e2.u.name) == 0);
            default:
                return 0;
        }
    }
    return 0;
}

/* unit tests */

static void test_token_equals() {
    struct Element input_number_0 = {ELEMENT_NUMBER, {0}};
    struct Element input_number_0_0 = {ELEMENT_NUMBER, {0}};
    struct Element input_number_1 = {ELEMENT_NUMBER, {1}};
    struct Element input_literal_name_0 = {ELEMENT_LITERAL_NAME, .u.name = "a"};
    struct Element input_literal_name_0_0 = {ELEMENT_LITERAL_NAME, .u.name = "a"};
    struct Element input_literal_name_1 = {ELEMENT_LITERAL_NAME, .u.name = "b"};

    assert(token_equals(input_number_0, input_number_0_0));
    assert(!token_equals(input_number_0, input_number_1));

    assert(token_equals(input_literal_name_0, input_literal_name_0_0));
    assert(!token_equals(input_literal_name_0, input_literal_name_1));
}

static void test_stack_pop() {
    struct Element *actual = stack_pop();

    assert(actual == NULL);
}

static void test_stack_push() {
    struct Element input = {0};

    stack_push(&input);

    assert(sp == 1);
    assert(token_equals(stack[0], input));
}

static void test_stack_push_pop() {
    struct Element input = {0};

    struct Element actual;

    stack_push(&input);
    actual = *stack_pop();

    assert(sp == 0);
    assert(token_equals(actual, input));
}

static void test_stack_push_push_pop_pop() {
    struct Element input_1 = {ELEMENT_NUMBER, {0}};
    struct Element input_2 = {ELEMENT_NUMBER, {1}};

    struct Element actual;

    stack_push(&input_1);
    stack_push(&input_2);

    actual = *stack_pop();
    assert(token_equals(actual, input_2));

    actual = *stack_pop();
    assert(token_equals(actual, input_1));
}


__attribute__((unused))
static void test_all() {
    stack_clear();
    test_token_equals();

    stack_clear();
    test_stack_pop();

    stack_clear();
    test_stack_push();

    stack_clear();
    test_stack_push_pop();

    stack_clear();
    test_stack_push_push_pop_pop();


    stack_push(&(struct Element){ELEMENT_NUMBER, .u.number = 123});
    stack_push(&(struct Element){ELEMENT_NUMBER, .u.number = 45});
    stack_push(&(struct Element){ELEMENT_LITERAL_NAME, .u.name = "some"});
    stack_push(&(struct Element){ELEMENT_LITERAL_NAME, .u.name = "some2"});

    stack_print_all();
}

#if 0
int main() {
    test_all();

    return 1;
}
#endif
