#include <assert.h>
#include <string.h>
#include "clesson.h"


#define assert_fail(msg) assert(0&&(msg))


static int streq(char *s1, char *s2) {
    return (strcmp(s1, s2) == 0);
}

static int stack_pop_number_value() {
    struct Element *el = stack_pop();
    if(!el){
        assert_fail("STACKUNDERFLOW");
    }

    if(ELEMENT_NUMBER != el->etype){
        assert_fail("TYPECHECK");
    }

    return el->u.number;
}

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
                case LEX_EXECUTABLE_NAME:
                    if(streq(token.u.name, "add")){
                        int i1 = stack_pop_number_value();
                        int i2 = stack_pop_number_value();
                        struct Element el = {ELEMENT_NUMBER, .u.number = i1 + i2};
                        stack_push(&el);
                    }
                    break;
                case LEX_SPACE:
                    break;
                default:
                    assert_fail("NOTIMPLEMENTED");
                    break;
            }
        }
    } while(ch != EOF);
}


static int verify_element_number(int expect, struct Element *el) {
    return (el
            && ELEMENT_NUMBER == el->etype
            && expect == el->u.number);
}


static void test_eval_num_one() {
    char *input = "123";
    int expect = 123;

    cl_getc_set_src(input);
    stack_clear();

    eval();

    assert(verify_element_number(expect, stack_pop()));
}

static void test_eval_num_two() {
    char *input = "123 456";
    int expect1 = 456;
    int expect2 = 123;

    cl_getc_set_src(input);
    stack_clear();

    eval();

    assert(verify_element_number(expect1, stack_pop()));
    assert(verify_element_number(expect2, stack_pop()));
}


static void test_eval_num_add() {
    char *input = "1 2 add";
    int expect = 3;

    cl_getc_set_src(input);
    stack_clear();

    eval();

    assert(verify_element_number(expect, stack_pop()));
}


int main() {
    test_eval_num_one();
    test_eval_num_two();
    test_eval_num_add();

    cl_getc_set_src("1 2 3 add add 4 5 6 7 8 9 add add add add add add");
    stack_clear();

    eval();

    assert(verify_element_number(45, stack_pop()));

    return 0;
}
