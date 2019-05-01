#include <stdio.h>
#include <assert.h>
#include "stack.h"


static void test_stack_pop() {

    struct Token *actual = stack_pop();

    assert(actual == NULL);
}

static void test_stack_push() {
    //assert(0);
}

static void test_stack_push_pop() {
    //assert(0);
}

static void test_stack_push_push_pop_pop() {
    //assert(0);
}

int main() {

    test_stack_pop();
    test_stack_push();
    test_stack_push_pop();
    test_stack_push_push_pop_pop();

    return 1;
}
