#include <assert.h>
#include "clesson.h"
#include "stack.h"

void eval() {}

static void assert_element_number(int expect, struct Element *el) {
    assert(NULL != el);
    assert(ELEMENT_NUMBER == el->etype);
    assert(expect == el->u.number);
}

static void test_eval_num_one() {
    char *input = "123";

    cl_getc_set_src(input);

    eval();

    assert_element_number(123, stack_pop());
}

static void test_eval_num_two() {
    char *input = "123 456";
    int expect1 = 456;
    int expect2 = 123;

    cl_getc_set_src(input);

    eval();

    /* TODO: write code to pop stack top and second top element */
    int actual1 = 0;
    int actual2 = 0;

    assert(expect1 == actual1);
    assert(expect2 == actual2);
}


static void test_eval_num_add() {
    char *input = "1 2 add";
    int expect = 3;

    cl_getc_set_src(input);

    eval();

    /* TODO: write code to pop stack top element */
    int actual = 0;
    assert(expect == actual);
}


int main() {
    stack_clear();
    test_eval_num_one();

    stack_clear();
    test_eval_num_two();

    stack_clear();
    test_eval_num_add();

    return 0;
}
