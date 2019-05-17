#include <assert.h>
#include "stack.h"
#include "dict.h"
#include "auto_element_array.h"
#include "eval.h"
#include "operator.h"


static void verify_op(const char *input_src, void(*op)(), const char *expect_src);


static void def_op() {
    Element *val = stack_pop();
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

static void mod_op() {
    int op2 = stack_pop_number();
    int res = stack_pop_number() % op2;
    stack_push_number(res);
}

static void eq_op() {
    int res = stack_pop_number() == stack_pop_number();
    stack_push_number(res);
}

static void test_eq() {
    verify_op("0 0", eq_op, "1");
    verify_op("0 0", eq_op, "1");
    verify_op("0 1", eq_op, "0");
    verify_op("1 0", eq_op, "0");
}


static void neq_op() {
    int res = stack_pop_number() != stack_pop_number();
    stack_push_number(res);
}

static void test_neq() {
    verify_op("0 0", neq_op, "0");
    verify_op("0 1", neq_op, "1");
    verify_op("1 0", neq_op, "1");
}


static void gt_op() {
    int op2 = stack_pop_number();
    int res = stack_pop_number() > op2;
    stack_push_number(res);
}

static void test_gt() {
    verify_op("0 0", gt_op, "0");
    verify_op("0 1", gt_op, "0");
    verify_op("1 0", gt_op, "1");
}


static void ge_op() {
    int op2 = stack_pop_number();
    int res = stack_pop_number() >= op2;
    stack_push_number(res);
}

static void test_ge() {
    verify_op("0 0", ge_op, "1");
    verify_op("0 1", ge_op, "0");
    verify_op("1 0", ge_op, "1");
}


static void lt_op() {
    int op2 = stack_pop_number();
    int res = stack_pop_number() < op2;
    stack_push_number(res);
}

static void test_lt() {
    verify_op("0 0", lt_op, "0");
    verify_op("0 1", lt_op, "1");
    verify_op("1 0", lt_op, "0");
}


static void le_op() {
    int op2 = stack_pop_number();
    int res = stack_pop_number() <= op2;
    stack_push_number(res);
}

static void test_le() {
    verify_op("0 0", le_op, "1");
    verify_op("0 1", le_op, "1");
    verify_op("1 0", le_op, "0");
}


static void pop_op() {
    stack_pop();
}

static void test_pop() {
    verify_op("1 2", pop_op, "1");
}


static void exch_op() {
    Element op2 = *stack_pop();
    Element op1 = *stack_pop();
    stack_push(&op2);
    stack_push(&op1);
}

static void test_exch() {
    verify_op("1 2", exch_op, "2 1");
}


static void dup_op() {
    Element op = *stack_pop();
    stack_push(&op);
    stack_push(&op);
}

static void test_dup() {
    verify_op("1", dup_op, "1 1");
}


static void index_op() {
    int n = stack_pop_number();
    AutoElementArray buf = {0};
    auto_element_array_init(&buf);

    while(n--) {
        Element *el = stack_pop();
        auto_element_array_add_element(&buf, el);
    }

    Element el = *stack_pop();
    stack_push(&el);

    ElementArray *arr = buf.var_array;
    int i = arr->len;
    while(i--) {
        stack_push(&arr->elements[i]);
    }

    stack_push(&el);
}

static void test_index() {
    verify_op("1 2 3 0", index_op, "1 2 3 3");
    verify_op("1 2 3 1", index_op, "1 2 3 2");
    verify_op("1 2 3 2", index_op, "1 2 3 1");
}


static void roll_op() {
    int j = stack_pop_number();
    int n = stack_pop_number();
    AutoElementArray buf = {0};
    auto_element_array_init(&buf);

    int i = n;
    while(i--) {
        Element *el = stack_pop();
        auto_element_array_add_element(&buf, el);
    }

    i = n;
    while(i--) {
        int k = (((i + j) % n) + n) % n;
        Element *el = &buf.var_array->elements[k];
        stack_push(el);
    }
}


static void test_roll() {
    verify_op("1 2 3  0 0", roll_op, "1 2 3");
    verify_op("1 2 3  1 0", roll_op, "1 2 3");
    verify_op("1 2 3  1 1", roll_op, "1 2 3");
    verify_op("1 2 3  2 0", roll_op, "1 2 3");


    verify_op("1 2 3  2 1", roll_op, "1 3 2");
    verify_op("1 2 3  2 2", roll_op, "1 2 3");
    verify_op("1 2 3  2 3", roll_op, "1 3 2");

    verify_op("1 2 3  3 -4", roll_op, "2 3 1");
    verify_op("1 2 3  3 -3", roll_op, "1 2 3");
    verify_op("1 2 3  3 -2", roll_op, "3 1 2");
    verify_op("1 2 3  3 -1", roll_op, "2 3 1");
    verify_op("1 2 3  3  0", roll_op, "1 2 3");
    verify_op("1 2 3  3  1", roll_op, "3 1 2");
    verify_op("1 2 3  3  2", roll_op, "2 3 1");
    verify_op("1 2 3  3  3", roll_op, "1 2 3");
    verify_op("1 2 3  3  4", roll_op, "3 1 2");
}


static void exec_op() {
    ElementArray *ea = stack_pop_exec_array();
    eval_exec_array(ea);
}

static void test_exec() {
    verify_op("/a {111 222 333} def /b {11 {a} exec 22 } def {1 b 2}", exec_op, "1 11 111 222 333 22 2");
}


static void if_op() {
    ElementArray *ea = stack_pop_exec_array();
    int is_true = stack_pop_number();

    if(is_true) {
        eval_exec_array(ea);
    }
}

static void test_if() {
    verify_op("8 1 {9}", if_op, "8 9");
    verify_op("8 0 {9}", if_op, "8");
}


static void ifelse_op() {
    ElementArray *proc_false = stack_pop_exec_array();
    ElementArray *proc_true = stack_pop_exec_array();
    int n = stack_pop_number();

    eval_exec_array(n? proc_true: proc_false);
}

static void test_ifelse() {
    verify_op("1 {9} {8}", ifelse_op, "9");
    verify_op("0 {9} {8}", ifelse_op, "8");
}

static void while_op() {
    ElementArray *body = stack_pop_exec_array();
    ElementArray *cond = stack_pop_exec_array();

    eval_exec_array(cond);
    while (stack_pop_number()) {
        eval_exec_array(body);
        eval_exec_array(cond);
    }
}

static void test_while() {
    verify_op("1 2 3 {0} {9}", while_op, "1 2 3");
    verify_op("1 2 3 {} {9 0}", while_op, "1 2 9");
    verify_op("1 0 2 3 {} {}", while_op, "1");
}


void register_primitive() {
    dict_put_c_func("def", def_op);

    dict_put_c_func("add", add_op);
    dict_put_c_func("sub", sub_op);
    dict_put_c_func("mul", mul_op);
    dict_put_c_func("div", div_op);
    dict_put_c_func("mod", mod_op);

    dict_put_c_func("eq", eq_op);
    dict_put_c_func("neq", neq_op);
    dict_put_c_func("gt", gt_op);
    dict_put_c_func("ge", ge_op);
    dict_put_c_func("lt", lt_op);
    dict_put_c_func("le", le_op);

    dict_put_c_func("pop", pop_op);
    dict_put_c_func("exch", exch_op);
    dict_put_c_func("dup", dup_op);
    dict_put_c_func("index", index_op);
    dict_put_c_func("roll", roll_op);

    dict_put_c_func("exec", exec_op);
    dict_put_c_func("if", if_op);
    dict_put_c_func("ifelse", ifelse_op);
    dict_put_c_func("while", while_op);
}


void operator_test_all() {

    test_eq();
    test_neq();
    test_gt();
    test_ge();
    test_lt();
    test_le();

    test_pop();
    test_exch();
    test_dup();
    test_index();
    test_roll();

    test_exec();
    test_if();
    test_ifelse();
    test_while();
}


static void verify_op(const char *input_src, void(*op)(), const char *expect_src) {
    call_eval(input_src);
    op();

    AutoElementArray actual = {0};
    auto_element_array_init(&actual);

    Element *el;
    while ((el = try_stack_pop())) {
       auto_element_array_add_element(&actual, el);
    }

    call_eval(expect_src);

    AutoElementArray expect = {0};
    auto_element_array_init(&expect);

    while((el = try_stack_pop())) {
        auto_element_array_add_element(&expect, el);
    }

    int eq = element_array_equals(expect.var_array, actual.var_array);
    assert(eq);
}
