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

    if(ELEMENT_LITERAL_NAME != el->etype) {
        assert_fail("NOT LITERAL_NAME");
    }

    return el->u.name;
}

static struct ElementArray *stack_pop_exec_array() {
    struct Element *el = stack_pop();

    if(ELEMENT_EXEC_ARRAY != el->etype) {
        assert_fail("NOT EXEC ARRAY");
    }

    return el->u.exec_array;
}

/* primitive */

static void def_op() {
    struct Element *val = stack_pop();
    char *name = stack_pop_literal_name();
    dict_put(name, val);
}

static void add_op() {
    int res = stack_pop_number_value() + stack_pop_number_value();
    stack_push(&(struct Element){ELEMENT_NUMBER, .u.number = res});
}

static void sub_op() {
    int op2 = stack_pop_number_value();
    int res = stack_pop_number_value() - op2;
    stack_push(&(struct Element){ELEMENT_NUMBER, .u.number = res});
}

static void mul_op() {
    int res = stack_pop_number_value() * stack_pop_number_value();
    stack_push(&(struct Element){ELEMENT_NUMBER, .u.number = res});
}

static void div_op() {
    int op2 = stack_pop_number_value();
    int res = stack_pop_number_value() / op2;
    stack_push(&(struct Element){ELEMENT_NUMBER, .u.number = res});
}

static void register_primitive() {
    dict_put("add", &(struct Element){ELEMENT_C_FUNC, .u.cfunc = &add_op});
    dict_put("def", &(struct Element){ELEMENT_C_FUNC, .u.cfunc = &def_op});
    dict_put("sub", &(struct Element){ELEMENT_C_FUNC, .u.cfunc = &sub_op});
    dict_put("mul", &(struct Element){ELEMENT_C_FUNC, .u.cfunc = &mul_op});
    dict_put("div", &(struct Element){ELEMENT_C_FUNC, .u.cfunc = &div_op});
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
                    {
                        struct Element el = {0};
                        if(!dict_get(token.u.name, &el)) {
                            assert_fail("EXECUTABLE NAME NOT FOUND");
                            break;
                        }

                        switch(el.etype) {
                            case ELEMENT_C_FUNC:
                                el.u.cfunc();
                                break;
                            default:
                                stack_push(&el);
                                break;
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
    register_primitive();
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

static void test_eval_sub() {
    char *input = "5 3 sub";
    int expect = 2;

    env_init(input);

    eval();
    assert(expect == stack_pop_number_value());
}

static void test_eval_mul() {
    char *input = "2 4 mul";
    int expect = 8;

    env_init(input);

    eval();
    assert(expect == stack_pop_number_value());
}

static void test_eval_div() {
    char *input = "6 2 div";
    int expect = 3;

    env_init(input);

    eval();
    assert(expect == stack_pop_number_value());
}

static void test_eval_div_truncation() {
    char *input = "14 4 div";
    int expect = 3;

    env_init(input);

    eval();
    assert(expect == stack_pop_number_value());
}

static void call_eval(char *input) {
    env_init(input);
    eval();
}

#define TO_EA(a) new_element_array(sizeof(a), a)

static void test_eval_executable_array_num() {
    char *input = "{1}";
    struct Element expect[] = {
        (struct Element){ELEMENT_NUMBER, .u.number = 1}
    };

    call_eval(input);
    assert(element_array_equals(TO_EA(expect), stack_pop_exec_array()));
}

static void test_eval_executable_array_literal_name() {
    char *input = "{/abc}";
    struct Element expect[] = {
        (struct Element){ELEMENT_LITERAL_NAME, .u.name = "abc"}
    };

    call_eval(input);
    assert(element_array_equals(TO_EA(expect), stack_pop_exec_array()));
}

static void test_eval_executable_array_executable_name() {
    char *input = "{abc}";
    struct Element expect[] = {
        (struct Element){ELEMENT_EXECUTABLE_NAME, .u.name = "abc"}
    };

    call_eval(input);
    assert(element_array_equals(TO_EA(expect), stack_pop_exec_array()));
}

static void test_eval_executable_array_multiple_elements() {
    char *input = "{1 2}";
    struct Element expect[] = {
        (struct Element){ELEMENT_NUMBER, .u.number = 1},
        (struct Element){ELEMENT_NUMBER, .u.number = 2}
    };

    call_eval(input);
    assert(element_array_equals(TO_EA(expect), stack_pop_exec_array()));
}

static void test_eval_executable_array_multiple_arrays() {
    char *input = "{1} {2}";
    struct Element expect1[] = {
        (struct Element){ELEMENT_NUMBER, .u.number = 1}
    };
    struct Element expect2[] = {
        (struct Element){ELEMENT_NUMBER, .u.number = 2}
    };

    call_eval(input);
    assert(element_array_equals(TO_EA(expect1), stack_pop_exec_array()));
    assert(element_array_equals(TO_EA(expect2), stack_pop_exec_array()));
}

static void test_eval_executable_array_nested() {
    char *input = "{1 {2} 3}";
    struct Element expect[] = {
        (struct Element){ELEMENT_NUMBER, .u.number = 1},
        (struct Element){ELEMENT_EXEC_ARRAY, .u.exec_array
            = TO_EA(((struct Element [1]){
                        (struct Element){ELEMENT_NUMBER, .u.number = 2}
                        }))},
        (struct Element){ELEMENT_NUMBER, .u.number = 3}
    };

    call_eval(input);
    assert(element_array_equals(TO_EA(expect), stack_pop_exec_array()));
}

#undef EA

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

        test_eval_sub();
        test_eval_mul();
        test_eval_div();
        test_eval_div_truncation();

        test_eval_executable_array_num();
        test_eval_executable_array_literal_name();
        test_eval_executable_array_executable_name();
        test_eval_executable_array_multiple_elements();
        test_eval_executable_array_multiple_arrays();
        test_eval_executable_array_nested();
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
