#include "def.h"
#include "util.h"
#include "parser.h"
#include "stack.h"
#include "dict.h"
#include "element.h"
#include "auto_element_array.h"
#include "c_operator.h"
#include "compiler.h"

#include "call_stack.h"
#include "eval.h"


static Element to_element(const Token *token);

void eval() {
    int ch = EOF;
    Token token = {0};

    do {
        ch = parse_one(ch, &token);
        if(token.ltype != LEX_UNKNOWN) {
            switch(token.ltype) {
                case LEX_NUMBER:
                case LEX_LITERAL_NAME:
                    {
                        Element el = to_element(&token);
                        stack_push(&el);
                    }
                    break;
                case LEX_EXECUTABLE_NAME:
                    {
                        Element el = {0};
                        if(!dict_get(token.u.name, &el)) {
                            assert_fail("EXECUTABLE NAME NOT FOUND");
                            return;
                        }

                        switch(el.etype) {
                            case ELEMENT_C_FUNC:
                                el.u.c_func();
                                break;
                            case ELEMENT_EXEC_ARRAY:
                                eval_exec_array(exec_exec_array, el.u.exec_array);
                                break;
                            default:
                                stack_push(&el);
                                break;
                        }
                    }
                    break;
                case LEX_OPEN_CURLY:
                    {
                        Element el = compile_exec_array(ch, &ch);
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

void exec_exec_array(Continuation *co) {
    const ElementArray *ea = co->exec_array;

    for(; co->pc < ea->len; co->pc++) {
        const Element *el = &ea->elements[co->pc];

        switch(el->etype) {
            case ELEMENT_NUMBER:
            case ELEMENT_LITERAL_NAME:
            case ELEMENT_EXEC_ARRAY:
                stack_push(el);
                break;
            case ELEMENT_PRIMITIVE:
                switch(el->u.operator) {
                    case OP_LPOP:
                        local_variable_pop();
                        break;
                    case OP_STORE:
                        {
                            Element el2 = {0};
                            stack_pop_element(&el2);
                            local_variable_push(&el2);
                        }
                        break;
                    case OP_LOAD:
                        {
                            int offset = stack_pop_number();
                            Element el2 = {0};
                            local_variable_get(offset, &el2);
                            stack_push(&el2);
                        }
                        break;
                    case OP_JMP:
                        {
                            int n = stack_pop_number();
                            co->pc += n;
                            if(co->pc < 0) {
                                co->pc = 0;
                            }
                            co->pc--;
                        }
                        break;
                    case OP_JMP_NOT_IF:
                        {
                            int n = stack_pop_number();
                            if(!stack_pop_number()) {
                                co->pc += n;
                                if(co->pc < 0){
                                    co->pc = 0;
                                }
                                co->pc--;
                            }
                        }
                        break;
                    case OP_EXEC:
                        {
                            ElementArray *exec_array = stack_pop_exec_array();
                            call_exec_array(exec_array, co);
                            return;
                        }
                }
                break;
            case ELEMENT_EXECUTABLE_NAME:
                {
                    Element el2 = {0};
                    if(!dict_get(el->u.name, &el2)) {
                        assert_fail("EXECUTABLE NAME NOT FOUND");
                        return;
                    }

                    switch(el2.etype) {
                        case ELEMENT_C_FUNC:
                            el2.u.c_func();
                            break;
                        case ELEMENT_EXEC_ARRAY:
                            call_exec_array(el2.u.exec_array, co);
                            return;
                        default:
                            stack_push(&el2);
                            break;
                    }
                }
                break;
            default:
                assert_fail("NOT IMPLEMENTED");
                break;
        }
    }
}


static Element to_element(const Token *token) {
    Element el = {0};

    switch(token->ltype) {
        case LEX_NUMBER:
            el.etype = ELEMENT_NUMBER;
            el.u.number = token->u.number;
            break;
        case LEX_EXECUTABLE_NAME:
            el.etype = ELEMENT_EXECUTABLE_NAME;
            el.u.name = token->u.name;
            break;
        case LEX_LITERAL_NAME:
            el.etype = ELEMENT_LITERAL_NAME;
            el.u.name = token->u.name;
            break;
        default:
            assert_fail("CAN'T CONVERT");
            break;
    }

    return el;
}


static void env_init() {
    stack_clear();
    dict_clear();
    compile_dict_clear();
    register_primitive();
    register_c_operator();
}


void call_eval(const char *input) {
    env_init();
    cl_getc_set_src(input);
    eval();
}


/* unit tests */

static void assert_dict_contains_number(const char *expect_key, int expect_number) {
    Element actual = {0};

    int exists = dict_get(expect_key, &actual);
    int eq_type = ELEMENT_NUMBER == actual.etype;
    int eq_number = expect_number == actual.u.number;

    assert(exists && eq_type && eq_number);
}

static void assert_stack(int expect_length, const Element expect_elements[]){
    Element el = {0};
    int i;
    for(i = 0; i < expect_length; i++) {
        stack_pop_element(&el);
        int eq = element_equals(&expect_elements[i], &el);
        assert(eq);
    }

    int empty = !try_stack_peek(NULL);
    assert(empty);
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

static void assert_pop_exec_array(int expect_len, const Element *expect_array) {
    ElementArray *expect = new_element_array_from_fixed_array(expect_len, expect_array);
    ElementArray *actual = stack_pop_exec_array();

    int eq = element_array_equals(expect, actual);
    assert(eq);
}

static void verify_stack_pop_exec_array(const char *input, int expect_len, const Element *expect_array) {
    call_eval(input);
    assert_pop_exec_array(expect_len, expect_array);
}


#define NEW_ELEMENT_ARRAY(a) new_element_array_from_fixed_array(sizeof(a) / sizeof(Element), a)

#define ASSERT_POP_EXEC_ARRAY(a) assert_pop_exec_array(sizeof(a) / sizeof(Element), a)

#define VERIFY_STACK_POP_EXEC_ARRAY(a, b) verify_stack_pop_exec_array(a, sizeof(b) / sizeof(Element), b)

#define ASSSERT_STACK(a) assert_stack(sizeof(a) / sizeof(Element), a)


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
    Element expect[] = {};

    VERIFY_STACK_POP_EXEC_ARRAY(input, expect);
}

static void test_compile_executable_array_num() {
    char *input = "{1}";
    Element expect[] = {
        {ELEMENT_NUMBER, .u.number = 1}
    };

    VERIFY_STACK_POP_EXEC_ARRAY(input, expect);
}

static void test_compile_executable_array_literal_name() {
    char *input = "{/abc}";
    Element expect[] = {
        {ELEMENT_LITERAL_NAME, .u.name = "abc"}
    };

    VERIFY_STACK_POP_EXEC_ARRAY(input, expect);
}

static void test_compile_executable_array_executable_name() {
    char *input = "{abc}";
    Element expect[] = {
        {ELEMENT_EXECUTABLE_NAME, .u.name = "abc"}
    };

    VERIFY_STACK_POP_EXEC_ARRAY(input, expect);
}

static void test_compile_executable_array_multiple_elements() {
    char *input = "{1 2}";
    Element expect[] = {
        {ELEMENT_NUMBER, .u.number = 1},
        {ELEMENT_NUMBER, .u.number = 2}
    };

    VERIFY_STACK_POP_EXEC_ARRAY(input, expect);
}


static void test_compile_executable_array_multiple_arrays() {
    char *input = "{1} {2}";
    Element expect1[] = {
        {ELEMENT_NUMBER, .u.number = 2}
    };
    Element expect2[] = {
        {ELEMENT_NUMBER, .u.number = 1}
    };

    call_eval(input);

    ASSERT_POP_EXEC_ARRAY(expect1);
    ASSERT_POP_EXEC_ARRAY(expect2);
    assert_stack(0, NULL);
}

static void test_compile_executable_array_nested() {
    char *input = "{1 {2} 3}";
    Element expect_contents[] = {
        {ELEMENT_NUMBER, .u.number = 2}
    };
    Element expect[] = {
        {ELEMENT_NUMBER, .u.number = 1},
        {ELEMENT_EXEC_ARRAY, .u.exec_array = NEW_ELEMENT_ARRAY(expect_contents)},
        {ELEMENT_NUMBER, .u.number = 3}
    };

    VERIFY_STACK_POP_EXEC_ARRAY(input, expect);
}

static void test_eval_executable_array() {
    verify_stack_pop_number("/ZZ {6} def ZZ", 6);
}


static void verify_eval(const char *input, const char *expect_input) {

    AutoElementArray actual = {0};
    auto_element_array_init(&actual);

    Element el = {0};

    call_eval(input);
    while (try_stack_pop_element(&el)) {
        auto_element_array_add_element(&actual, &el);
    }

    AutoElementArray expect = {0};
    auto_element_array_init(&expect);

    call_eval(expect_input);
    while (try_stack_pop_element(&el)) {
        auto_element_array_add_element(&expect, &el);
    }

    int eq = element_array_equals(expect.var_array, actual.var_array);
    assert(eq);
}

static void test_eval_executable_array_nested_nested() {
    char *input = "/ZZ {6} def " \
                   "/YY {4 ZZ 5} def " \
                   "/XX {1 2 YY 3} def " \
                   " XX";
    verify_eval(input, "1 2 4 6 5 3");
}

static void test_while() {
    verify_eval("3 {dup 1 sub dup 1 ge} {9 exch} while", "3 9 2 9 1 0");
    verify_eval("5  { dup 10 lt } { 3 add } while", "11");
    verify_eval("4 dup {dup 1 gt} {1 sub exch 1 index mul exch} while pop", "24");
    verify_eval("1 { 2 add dup 4 lt } { 10 add } while", "15");

    verify_eval("{ 3 {dup 1 sub dup 1 ge} {9 exch} while} exec", "3 9 2 9 1 0");

    verify_eval("1 dup {dup 1 gt} exec", "1 1 0");
    verify_eval("3 3{1 sub exch 1 index mul exch} exec", "6 2");
    verify_eval("3 dup {dup 1 gt} exec", "3 3 1");
    verify_eval("/f{ dup {dup 1 gt} { 1 sub exch 1 index mul exch} while pop } def 1 f", "1");
}

static void test_eval_exec_array() {
    verify_eval("/a {0} def /b {{a 1} exec 2} def {b 3} exec", "0 1 2 3");
    verify_eval("/a {1} def /b {0 a {1}if 2} def {b 3} exec", "0 1 2 3");
    verify_eval("/a {0} def /b {0 a {4}{1}ifelse 2} def {b 3} exec", "0 1 2 3");
}

static void test_eval_exec_array_jmp() {
    verify_eval("{1 jmp 1 2 3 4 5} exec 10", "1 2 3 4 5 10");
    verify_eval("{3 jmp 1 2 3 4 5} exec 10", "3 4 5 10");
    verify_eval("{10 jmp 1 2 3 4 5} exec 10", "10");
    verify_eval("2 {jmp 123 5 -10 jmp 9} exec 10", "9 10");
    verify_eval("2 {jmp 123 5 -4 jmp 9} exec 10", "9 10");
}

static void test_eval_exec_array_primitive() {
    verify_eval("{1 store 2 store 1 load 0 load 1 load 0 load add } exec 4", "1 2 3 4");
    verify_eval("/a {1} def {a {a} repeat 2 {2} repeat 3} exec 4", "1 2 2 3 4");
}

void eval_test_all() {

    verify_eval("{1 0 1 1 {}{} while 2 } exec 3", "1 2 3");

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

    test_while();

    verify_stack_pop_number("1 2 3 add add 4 5 6 7 8 9 add add add add add add", 45);


    verify_eval("1 {2} {3} ifelse 4", "2 4");
    verify_eval("/a { {345} ifelse} def 1 {123} a", "123");
    verify_eval("/f {{1 3 add} exec 3} def f", "4 3");

    verify_eval("{1 {2} {3} ifelse} exec 4", "2 4");

    verify_eval("/a {{0} {1} ifelse} def /b {{1 a} {0 a}ifelse} def /c {1 b} def c", "0");

    test_eval_exec_array();
    test_eval_exec_array_jmp();
    test_eval_exec_array_primitive();
}

#if 1
int main(int argc, char *argv[]) {

    if(argc <= 1) {
        parser_test_all();
        stack_test_all();
        dict_test_all();
        element_test_all();
        element_array_test_all();
        auto_element_array_test_all();
        operator_test_all();
        eval_test_all();
        return 0;
    }

    FILE *f = NULL;
    f = fopen(argv[1], "r");
    if(!f) {
        printf("ERROR! FILE CANNOT OPEN: [%s]", argv[1]);
        return 1;
    }

    cl_getc_set_file(f);
    env_init();
    eval();

    stack_print();


    return 0;
}
#endif
