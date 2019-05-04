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


/* unit tests */

static void test_stack_pop() {
    struct Element *actual = stack_pop();

    assert(actual == NULL);
}

static void test_stack_push() {
    struct Element input = {0};

    stack_push(&input);

    assert(sp == 1);
    assert(element_equals(stack[0], input));
}

static void test_stack_push_pop() {
    struct Element input = {0};

    struct Element actual;

    stack_push(&input);
    actual = *stack_pop();

    assert(sp == 0);
    assert(element_equals(actual, input));
}

static void test_stack_push_push_pop_pop() {
    struct Element input_1 = {ELEMENT_NUMBER, {0}};
    struct Element input_2 = {ELEMENT_NUMBER, {1}};

    struct Element actual;

    stack_push(&input_1);
    stack_push(&input_2);

    actual = *stack_pop();
    assert(element_equals(actual, input_2));

    actual = *stack_pop();
    assert(element_equals(actual, input_1));
}


__attribute__((unused))
static void test_all() {
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
