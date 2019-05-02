#include <assert.h>
#include "clesson.h"


void eval() {
    int ch = EOF;
    struct Token token = {
        LEX_UNKNOWN,
        {0}
    };

    do {
        ch = parse_one(ch, &token);
        if(token.ltype != LEX_UNKNOWN) {
            switch(token.ltype) {
                case LEX_NUMBER:
                    stack_push((struct Element*)&token);
                    break;
                default:
                    break;
            }
        }
    } while(ch != EOF);


}

static void assert_equals_element_number(int expect, struct Element *el) {
    assert(NULL != el);
    assert(ELEMENT_NUMBER == el->etype);
    assert(expect == el->u.number);
}

static void test_eval_num_one() {
    char *input = "123";

    cl_getc_set_src(input);
    stack_clear();

    eval();

    assert_equals_element_number(123, stack_pop());
}

static void test_eval_num_two() {
    char *input = "123 456";
    int expect1 = 456;
    int expect2 = 123;

    cl_getc_set_src(input);
    stack_clear();

    eval();

    assert_equals_element_number(expect1, stack_pop());
    assert_equals_element_number(expect2, stack_pop());
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
    test_eval_num_one();
    test_eval_num_two();
    test_eval_num_add();

    return 0;
}
