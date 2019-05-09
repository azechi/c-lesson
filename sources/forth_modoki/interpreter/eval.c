#include <assert.h>
#include "clesson.h"


static struct Element stack_pop() {
    struct Element *el = try_stack_pop();
    if(!el){
        assert_fail("STACKUNDERFLOW");
    }
    return *el;
}

static int stack_pop_number_value() {
    struct Element el = stack_pop();

    if(ELEMENT_NUMBER != el.etype){
        assert_fail("NOT NUMBER ELEMENT");
    }

    return el.u.number;
}

static char *stack_pop_literal_name() {
    struct Element el = stack_pop();

    if(ELEMENT_LITERAL_NAME != el.etype) {
        assert_fail("NOT LITERAL_NAME");
    }

    return el.u.name;
}

static struct ElementArray *stack_pop_exec_array() {
    struct Element el = stack_pop();

    if(ELEMENT_EXEC_ARRAY != el.etype) {
        assert_fail("NOT EXEC ARRAY");
    }

    return el.u.exec_array;
}

/* primitive */

static void def_op() {
    struct Element val = stack_pop();
    char *name = stack_pop_literal_name();
    dict_put(name, val);
}

static void add_op() {
    int res = stack_pop_number_value() + stack_pop_number_value();
    stack_push(element_number(res));
}

static void sub_op() {
    int op2 = stack_pop_number_value();
    int res = stack_pop_number_value() - op2;
    stack_push(element_number(res));
}

static void mul_op() {
    int res = stack_pop_number_value() * stack_pop_number_value();
    stack_push(element_number(res));
}

static void div_op() {
    int op2 = stack_pop_number_value();
    int res = stack_pop_number_value() / op2;
    stack_push(element_number(res));
}

static void register_primitive() {
    dict_put("def", element_c_func(def_op));
    dict_put("add", element_c_func(add_op));
    dict_put("sub", element_c_func(sub_op));
    dict_put("mul", element_c_func(mul_op));
    dict_put("div", element_c_func(div_op));
}

#define MAX_OP_NUMBERS 256
static void eval_executable_name(char *name);
static void eval_exec_array(struct ElementArray *ea);


static void eval_exec_array(struct ElementArray *ea) {
    int i;
    int len = ea->len;

    for(i = 0; i < len; i++) {
        struct Element el = ea->elements[i];

        switch(el.etype) {
            case ELEMENT_NUMBER:
            case ELEMENT_LITERAL_NAME:
                stack_push(el);
                break;
            case ELEMENT_C_FUNC:
                el.u.cfunc();
                break;
            case ELEMENT_EXEC_ARRAY:
                eval_exec_array(el.u.exec_array);
                break;
            case ELEMENT_EXECUTABLE_NAME:
                eval_executable_name(el.u.name);
                break;
            default:
                assert_fail("NOT IMPLEMENTED");
                break;
        }
    }
}

static void eval_executable_name(char *name) {
    struct Element el = {0};
    if(!dict_get(name, &el)) {
        assert_fail("EXECUTABLE NAME NOT FOUND");
        return;
    }

    switch(el.etype) {
        case ELEMENT_C_FUNC:
            el.u.cfunc();
            break;
        case ELEMENT_EXEC_ARRAY:
            eval_exec_array(el.u.exec_array);
            break;
        default:
            stack_push(el);
            break;
    }
}

static int compile_exec_array(int ch, struct Element *out_element) {
    struct Element elements[MAX_OP_NUMBERS];
    struct Token token = {LEX_UNKNOWN, {0}};

    int i = 0;
    do {
        ch = parse_one(ch, &token);
        switch(token.ltype) {
            case LEX_NUMBER:
            case LEX_EXECUTABLE_NAME:
            case LEX_LITERAL_NAME:
                elements[i++] = *(struct Element*)&token;
                break;
            case LEX_CLOSE_CURLY:
            case LEX_SPACE:
                break;
            case LEX_OPEN_CURLY:
                {
                    struct Element el = {0};
                    ch = compile_exec_array(ch, &el);
                    elements[i++] = el;
                }
                break;
            case LEX_END_OF_FILE:
            default:
                assert_fail("NOT IMPLEMENTED");
                break;

        }
    } while(token.ltype != LEX_CLOSE_CURLY);

    *out_element = element_exec_array(new_element_array(i, elements));
    return ch;
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
                    stack_push(*(struct Element*)&token);
                    break;
                case LEX_EXECUTABLE_NAME:
                    eval_executable_name(token.u.name);
                    break;
                case LEX_OPEN_CURLY:
                    {
                        struct Element el = {0};
                        ch = compile_exec_array(ch, &el);
                        stack_push(el);
                    }
                    break;
                case LEX_CLOSE_CURLY:
                    assert_fail("SYNTAX ERROR");
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


/* unit tests */

static void call_eval(char *input) {
    env_init(input);
    eval();
}

static void assert_dict_contains(char *expect_key, struct Element expect_el) {
    struct Element actual = {0};

    assert(dict_get(expect_key, &actual));
    assert(element_equals(expect_el, actual));
}

static void assert_stack(int expect_length, struct Element expect_elements[]){
    int i;
    for(i = 0; i < expect_length; i++) {
        assert(element_equals(expect_elements[i], stack_pop()));
    }
    assert(try_stack_pop() == NULL);
}

#define ELEMENT_ARRAY(a) new_element_array(sizeof(a) / sizeof(struct Element), a)

#define ASSERT_ELEMENT_ARRAY_EQUALS(a, b) assert(element_array_equals(ELEMENT_ARRAY(a), b))
#define ASSSERT_STACK(a) assert_stack(sizeof(a) / sizeof(struct Element), a)


static void test_stack_pop() {
    struct Element input = element_number(1);
    struct Element expect = element_number(1);

    stack_clear();
    stack_push(input);

    assert(element_equals(expect, stack_pop()));
}

static void test_eval_empty() {
    env_init("");
    assert(NULL == try_stack_pop());
}

static void test_eval_num_one() {
    char *input = "123";
    int expect = 123;

    call_eval(input);
    assert(expect == stack_pop_number_value());
}

static void test_eval_num_two() {
    char *input = "123 456";
    int expect1 = 456;
    int expect2 = 123;

    call_eval(input);
    assert(expect1 == stack_pop_number_value());
    assert(expect2 == stack_pop_number_value());
}

static void test_eval_num_add() {
    char *input = "1 2 add";
    int expect = 3;

    call_eval(input);
    assert(expect == stack_pop_number_value());
}

static void test_eval_literal_name() {
    char *input = "/abc";
    struct Element expect = element_literal_name("abc");

    call_eval(input);
    assert(element_equals(expect, stack_pop()));
}


static void test_eval_def_dict_put() {
    char *input = "/abc 123 def";
    char *expect_key = "abc";
    struct Element expect_el = element_number(123);

    call_eval(input);
    assert_dict_contains(expect_key, expect_el);
}

static void test_eval_def_dict_get() {
    char *input = "/abc 123 def abc";
    int expect = 123;

    call_eval(input);
    assert(expect == stack_pop_number_value());
}

static void test_eval_sub() {
    char *input = "5 3 sub";
    int expect = 2;

    call_eval(input);
    assert(expect == stack_pop_number_value());
}

static void test_eval_mul() {
    char *input = "2 4 mul";
    int expect = 8;

    call_eval(input);
    assert(expect == stack_pop_number_value());
}

static void test_eval_div() {
    char *input = "6 2 div";
    int expect = 3;

    call_eval(input);
    assert(expect == stack_pop_number_value());
}

static void test_eval_div_truncation() {
    char *input = "14 4 div";
    int expect = 3;

    call_eval(input);
    assert(expect == stack_pop_number_value());
}


static void test_compile_executable_array_empty() {
    char *input = "{}";
    struct Element expect[] = {};

    call_eval(input);
    ASSERT_ELEMENT_ARRAY_EQUALS(expect, stack_pop_exec_array());
}

static void test_compile_executable_array_num() {
    char *input = "{1}";
    struct Element expect[] = {
        element_number(1)
    };

    call_eval(input);
    ASSERT_ELEMENT_ARRAY_EQUALS(expect, stack_pop_exec_array());
}

static void test_compile_executable_array_literal_name() {
    char *input = "{/abc}";
    struct Element expect[] = {
        element_literal_name("abc")
    };

    call_eval(input);
    ASSERT_ELEMENT_ARRAY_EQUALS(expect, stack_pop_exec_array());
}

static void test_compile_executable_array_executable_name() {
    char *input = "{abc}";
    struct Element expect[] = {
        element_executable_name("abc")
    };

    call_eval(input);
    ASSERT_ELEMENT_ARRAY_EQUALS(expect, stack_pop_exec_array());
}

static void test_compile_executable_array_multiple_elements() {
    char *input = "{1 2}";
    struct Element expect[] = {
        element_number(1),
        element_number(2)
    };

    call_eval(input);
    ASSERT_ELEMENT_ARRAY_EQUALS(expect, stack_pop_exec_array());
}

static void test_compile_executable_array_multiple_arrays() {
    char *input = "{1} {2}";
    struct Element expect1[] = {
        element_number(2)
    };
    struct Element expect2[] = {
        element_number(1)
    };

    call_eval(input);
    ASSERT_ELEMENT_ARRAY_EQUALS(expect1, stack_pop_exec_array());
    ASSERT_ELEMENT_ARRAY_EQUALS(expect2, stack_pop_exec_array());
}

static void test_compile_executable_array_nested() {
    char *input = "{1 {2} 3}";
    struct Element expect_contents[] = {
        element_number(2)
    };
    struct Element expect[] = {
        element_number(1), 
        element_exec_array(ELEMENT_ARRAY(expect_contents)), 
        element_number(3)
    };

    call_eval(input);
    ASSERT_ELEMENT_ARRAY_EQUALS(expect, stack_pop_exec_array());
}

static void test_eval_executable_array() {
    char *input = "/ZZ {6} def ZZ";
    int expect = 6;

    call_eval(input);
    assert(expect == stack_pop_number_value());
}

static void test_eval_executable_array_nested() {
    char *input = "/ZZ {6} def /YY {4 ZZ} def";
    char *expect_key = "YY";
    struct Element expect_el_contents[] = {
        element_number(4),
        element_executable_name("ZZ")
    };
    struct Element expect_el = element_exec_array(ELEMENT_ARRAY(expect_el_contents));

    call_eval(input);
    assert_dict_contains(expect_key, expect_el);
}


static void test_eval_executable_array_nested_nested() {
    char *input = "/ZZ {6} def" \
                   "/YY {4 ZZ 5} def" \
                   "/XX {1 2 YY 3} def" \
                   " XX";
    struct Element expect[] = {
        element_number(3),
        element_number(5),
        element_number(6),
        element_number(4),
        element_number(2),
        element_number(1)
    };

    call_eval(input);
    ASSSERT_STACK(expect);
}


__attribute__((unused))
    static void test_all() {
        test_stack_pop();

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

        test_compile_executable_array_empty();
        test_compile_executable_array_num();
        test_compile_executable_array_literal_name();
        test_compile_executable_array_executable_name();
        test_compile_executable_array_multiple_elements();
        test_compile_executable_array_multiple_arrays();
        test_compile_executable_array_nested();

        test_eval_executable_array();
        test_eval_executable_array_nested();
        test_eval_executable_array_nested_nested();
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
