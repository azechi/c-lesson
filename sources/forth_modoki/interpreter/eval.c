#include <assert.h>
#include "clesson.h"


static struct Element *stack_pop() {
    struct Element *el = try_stack_pop();
    if(!el){
        assert_fail("STACKUNDERFLOW");
    }
    return el;
}

static int stack_pop_number() {
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

static void stack_push_number(int i) {
    struct Element el = {ELEMENT_NUMBER, .u.number = i};
    stack_push(&el);
}

static void dict_put_c_func(const char *name, void(*c_func)()) {
    struct Element el = {ELEMENT_C_FUNC, .u.cfunc = c_func};
    dict_put(name, &el);
}

/* primitive */

static void def_op() {
    struct Element *val = stack_pop();
    char *name = stack_pop_literal_name();
    dict_put(name, val);
}

static void add_op() {
    int res = stack_pop_number() + stack_pop_number();
    stack_push_number(res);
}

static void sub_op() {
    int op2 = stack_pop_number();
    int res = stack_pop_number() - op2;
    stack_push_number(res);
}

static void mul_op() {
    int res = stack_pop_number() * stack_pop_number();
    stack_push_number(res);
}

static void div_op() {
    int op2 = stack_pop_number();
    int res = stack_pop_number() / op2;
    stack_push_number(res);
}

static void register_primitive() {
    dict_put_c_func("def", def_op);
    dict_put_c_func("add", add_op);
    dict_put_c_func("sub", sub_op);
    dict_put_c_func("mul", mul_op);
    dict_put_c_func("div", div_op);
}

#define MAX_OP_NUMBERS 256
static void eval_executable_name(const char *name);
static void eval_exec_array(const struct ElementArray *ea);


static void eval_exec_array(const struct ElementArray *ea) {
    int i;
    int len = ea->len;

    for(i = 0; i < len; i++) {
        const struct Element *el = &ea->elements[i];

        switch(el->etype) {
            case ELEMENT_NUMBER:
            case ELEMENT_LITERAL_NAME:
                stack_push(el);
                break;
            case ELEMENT_C_FUNC:
                el->u.cfunc();
                break;
            case ELEMENT_EXEC_ARRAY:
                eval_exec_array(el->u.exec_array);
                break;
            case ELEMENT_EXECUTABLE_NAME:
                eval_executable_name(el->u.name);
                break;
            default:
                assert_fail("NOT IMPLEMENTED");
                break;
        }
    }
}

static void eval_executable_name(const char *name) {
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
            stack_push(&el);
            break;
    }
}

static struct Element to_element(const struct Token *token) {
    struct Element el = {0};

    switch(token->ltype) {
        case LEX_NUMBER:
            el.u.number = token->u.number;
            break;
        case LEX_EXECUTABLE_NAME:
        case LEX_LITERAL_NAME:
            el.u.name = token->u.name;
            break;
        default:
            assert_fail("CAN'T CONVERT");
            break;
    }

    el.etype = (enum ElementType)token->ltype;
    return el;
}

static struct Element compile_exec_array(int ch, int *out_ch) {
    struct Element elements[MAX_OP_NUMBERS];
    struct Token token = {LEX_UNKNOWN, {0}};

    int i = 0;
    do {
        ch = parse_one(ch, &token);
        switch(token.ltype) {
            case LEX_NUMBER:
            case LEX_EXECUTABLE_NAME:
            case LEX_LITERAL_NAME:
                elements[i++] = to_element(&token);
                break;
            case LEX_CLOSE_CURLY:
            case LEX_SPACE:
                break;
            case LEX_OPEN_CURLY:
                elements[i++] = compile_exec_array(ch, &ch);
                break;
            case LEX_END_OF_FILE:
            default:
                assert_fail("NOT IMPLEMENTED");
                break;
        }
    } while(token.ltype != LEX_CLOSE_CURLY);

    *out_ch = ch;

    struct Element el = {ELEMENT_EXEC_ARRAY, {0}};
    el.u.exec_array = new_element_array(i, elements);

    return el;
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
                    {
                        struct Element el = to_element(&token);
                        stack_push(&el);
                    }
                    break;
                case LEX_EXECUTABLE_NAME:
                    eval_executable_name(token.u.name);
                    break;
                case LEX_OPEN_CURLY:
                    {
                        struct Element el = compile_exec_array(ch, &ch);
                        stack_push(&el);
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

static void env_init(const char *input) {
    cl_getc_set_src(input);
    stack_clear();
    dict_clear();
    register_primitive();
}


/* unit tests */

static void call_eval(const char *input) {
    env_init(input);
    eval();
}

static void assert_dict_contains_number(const char *expect_key, int expect_number) {
    struct Element actual = {0};

    int exists = dict_get(expect_key, &actual);
    int eq_type = ELEMENT_NUMBER == actual.etype;
    int eq_number = expect_number == actual.u.number;

    assert(exists && eq_type && eq_number);
}

static void assert_stack(int expect_length, const struct Element expect_elements[]){
    const struct Element *el;
    int i;
    for(i = 0; i < expect_length; i++) {
        el = stack_pop();
        int eq = element_equals(&expect_elements[i], el);
        assert(eq);
    }

    el = try_stack_pop();
    assert(el == NULL);
}

static void assert_stack_pop_number(int expect) {
    int actual = stack_pop_number();
    assert(expect == actual);
}

static void assert_stack_pop_literal_name(const char *expect) {
    const char *actual = stack_pop_literal_name();
    int eq = streq(expect, actual);
    assert(eq);
}

static void verify_stack_pop_number(const char *input, int expect) {
    call_eval(input);

    int actual = stack_pop_number();
    assert(expect == actual);
    assert_stack(0, NULL);
}

static void assert_pop_exec_array(int expect_len, const struct Element *expect_array) {
    struct ElementArray *expect = new_element_array(expect_len, expect_array);
    struct ElementArray *actual = stack_pop_exec_array();

    int eq = element_array_equals(expect, actual);
    assert(eq);
}

static void verify_stack_pop_exec_array(const char *input, int expect_len, const struct Element *expect_array) {
    call_eval(input);
    assert_pop_exec_array(expect_len, expect_array);
}


#define NEW_ELEMENT_ARRAY(a) new_element_array(sizeof(a) / sizeof(struct Element), a)

#define ASSERT_POP_EXEC_ARRAY(a) assert_pop_exec_array(sizeof(a) / sizeof(struct Element), a)

#define VERIFY_STACK_POP_EXEC_ARRAY(a, b) verify_stack_pop_exec_array(a, sizeof(b) / sizeof(struct Element), b)

#define ASSSERT_STACK(a) assert_stack(sizeof(a) / sizeof(struct Element), a)


static void test_eval_empty() {
    call_eval("");
    assert_stack(0, NULL);
}

static void test_eval_num_one() {
    verify_stack_pop_number("123", 123);
}

static void test_eval_num_two() {
    call_eval("123 456");
    assert_stack_pop_number(456);
    assert_stack_pop_number(123);
}

static void test_eval_num_add() {
    verify_stack_pop_number("1 2 add", 3);
}

static void test_eval_literal_name() {
    call_eval("/abc");
    assert_stack_pop_literal_name("abc");
}

static void test_eval_def_dict_put() {
    call_eval("/abc 123 def");
    assert_dict_contains_number("abc", 123);
}

static void test_eval_def_dict_get() {
    verify_stack_pop_number("/abc 123 def abc", 123);
}

static void test_eval_sub() {
    verify_stack_pop_number("5 3 sub", 2);
}

static void test_eval_mul() {
    verify_stack_pop_number("2 4 mul", 8);
}

static void test_eval_div() {
    verify_stack_pop_number("6 2 div", 3);
}

static void test_eval_div_truncation() {
    verify_stack_pop_number("14 4 div", 3);
}


static void test_compile_executable_array_empty() {
    char *input = "{}";
    struct Element expect[] = {};

    VERIFY_STACK_POP_EXEC_ARRAY(input, expect);
}

static void test_compile_executable_array_num() {
    char *input = "{1}";
    struct Element expect[] = {
        {ELEMENT_NUMBER, .u.number = 1}
    };

    VERIFY_STACK_POP_EXEC_ARRAY(input, expect);
}

static void test_compile_executable_array_literal_name() {
    char *input = "{/abc}";
    struct Element expect[] = {
        {ELEMENT_LITERAL_NAME, .u.name = "abc"}
    };

    VERIFY_STACK_POP_EXEC_ARRAY(input, expect);
}

static void test_compile_executable_array_executable_name() {
    char *input = "{abc}";
    struct Element expect[] = {
        {ELEMENT_EXECUTABLE_NAME, .u.name = "abc"}
    };

    VERIFY_STACK_POP_EXEC_ARRAY(input, expect);
}

static void test_compile_executable_array_multiple_elements() {
    char *input = "{1 2}";
    struct Element expect[] = {
        {ELEMENT_NUMBER, .u.number = 1},
        {ELEMENT_NUMBER, .u.number = 2}
    };

    VERIFY_STACK_POP_EXEC_ARRAY(input, expect);
}


static void test_compile_executable_array_multiple_arrays() {
    char *input = "{1} {2}";
    struct Element expect1[] = {
        {ELEMENT_NUMBER, .u.number = 2}
    };
    struct Element expect2[] = {
        {ELEMENT_NUMBER, .u.number = 1}
    };

    call_eval(input);

    ASSERT_POP_EXEC_ARRAY(expect1);
    ASSERT_POP_EXEC_ARRAY(expect2);
    assert_stack(0, NULL);
}

static void test_compile_executable_array_nested() {
    char *input = "{1 {2} 3}";
    struct Element expect_contents[] = {
        {ELEMENT_NUMBER, .u.number = 2}
    };
    struct Element expect[] = {
        {ELEMENT_NUMBER, .u.number = 1},
        {ELEMENT_EXEC_ARRAY, .u.exec_array = NEW_ELEMENT_ARRAY(expect_contents)},
        {ELEMENT_NUMBER, .u.number = 3}
    };

    VERIFY_STACK_POP_EXEC_ARRAY(input, expect);
}

static void test_eval_executable_array() {
    verify_stack_pop_number("/ZZ {6} def ZZ", 6);
}

static void test_eval_executable_array_nested_nested() {
    char *input = "/ZZ {6} def" \
                   "/YY {4 ZZ 5} def" \
                   "/XX {1 2 YY 3} def" \
                   " XX";
    struct Element expect[] = {
        {ELEMENT_NUMBER, .u.number = 3},
        {ELEMENT_NUMBER, .u.number = 5},
        {ELEMENT_NUMBER, .u.number = 6},
        {ELEMENT_NUMBER, .u.number = 4},
        {ELEMENT_NUMBER, .u.number = 2},
        {ELEMENT_NUMBER, .u.number = 1}
    };

    call_eval(input);
    ASSSERT_STACK(expect);
}


__attribute__((unused))
    static void test_all() {

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
        test_eval_executable_array_nested_nested();
    }

#if 1
int main() {
    test_all();

    verify_stack_pop_number("1 2 3 add add 4 5 6 7 8 9 add add add add add add", 45);

    return 0;
}
#endif
