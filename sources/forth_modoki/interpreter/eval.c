#include <assert.h>
#include "clesson.h"


#define assert_fail(msg) assert(0&&(msg))

static struct Element *stack_pop() {
    struct Element *el = try_stack_pop();
    if(!el){
        assert_fail("STACKUNDERFLOW");
    }
    return el;
}

static int stack_pop_number_value() {
    struct Element *el = stack_pop();

    if(ELEMENT_NUMBER != el->etype){
        assert_fail("NOT NUMBER ELEMENT");
    }

    return el->u.number;
}

static char *stack_pop_literal_name() {
    struct Element *el = stack_pop();

    if(ELEMENT_LITERAL_NAME != el->etype){
        assert_fail("NOT LITERAL_NAME");
    }

    return el->u.name;
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
                case LEX_LITERAL_NAME:
                    stack_push((struct Element*)&token);
                    break;
                case LEX_EXECUTABLE_NAME:
                    if(streq(token.u.name, "add")){
                        int i1 = stack_pop_number_value();
                        int i2 = stack_pop_number_value();
                        struct Element el = {ELEMENT_NUMBER, .u.number = i1 + i2};
                        stack_push(&el);
                    } else if(streq(token.u.name, "def")) {
                        struct Element *val = stack_pop();
                        char *name = stack_pop_literal_name();
                        dict_put(name, val);
                    } else {
                        struct Element el = {0};
                        if(dict_get(token.u.name, &el)) {
                            stack_push(&el);
                        }
                    }
                    break;
                case LEX_SPACE:
                case LEX_END_OF_FILE:
                    break;
                default:
                    assert_fail("NOT IMPLEMENTED");
                    break;
            }
        }
    } while(ch != EOF);
}

static void env_init(char *input) {
    cl_getc_set_src(input);
    stack_clear();
    dict_clear();
}


static void assert_dict_contains(char *expect_key, struct Element *expect_el) {
    struct Element actual = {0};

    assert(dict_get(expect_key, &actual));
    assert(element_equals(*expect_el, actual));
}

static void test_pop_number_value() {
    struct Element input = {ELEMENT_NUMBER, {1}};
    int expect = 1;

    env_init("");

    stack_push(&input);

    assert(expect == stack_pop_number_value());
}

static void test_eval_empty() {
    char *input = "";

    env_init(input);

    eval();

    assert(NULL == try_stack_pop());
}

static void test_eval_num_one() {
    char *input = "123";
    int expect = 123;

    env_init(input);

    eval();

    assert(expect == stack_pop_number_value());
}

static void test_eval_num_two() {
    char *input = "123 456";
    int expect1 = 456;
    int expect2 = 123;

    env_init(input);

    eval();

    assert(expect1 == stack_pop_number_value());
    assert(expect2 == stack_pop_number_value());
}

static void test_eval_num_add() {
    char *input = "1 2 add";
    int expect = 3;

    env_init(input);

    eval();

    assert(expect == stack_pop_number_value());
}

static void test_eval_literal_name() {
    char *input = "/abc";
    struct Element expect = {ELEMENT_LITERAL_NAME, .u.name = "abc"};

    env_init(input);

    eval();
    assert(element_equals(expect, *stack_pop()));
}


static void test_eval_def_dict_put() {
    char *input = "/abc 123 def";
    char *expect_key = "abc";
    struct Element expect_el = {ELEMENT_NUMBER, .u.number = 123};

    env_init(input);

    eval();

    assert_dict_contains(expect_key, &expect_el);
}

static void test_eval_def_dict_get() {
    char *input = "/abc 123 def abc";
    int expect = 123;

    env_init(input);

    eval();
    assert(expect == stack_pop_number_value());
}

__attribute__((unused))
    static void test_all() {
        test_pop_number_value();
        test_eval_empty();
        test_eval_num_one();
        test_eval_num_two();
        test_eval_num_add();

        test_eval_literal_name();
        test_eval_def_dict_put();
        test_eval_def_dict_get();
    }

#if 1
int main() {
    test_all();

    env_init("1 2 3 add add 4 5 6 7 8 9 add add add add add add");

    eval();

    assert(45 == stack_pop_number_value());

    return 0;
}
#endif
