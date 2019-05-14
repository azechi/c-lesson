#include <stdlib.h>
#include <assert.h>

#include "util.h"
#include "element.h"


int element_array_equals(const ElementArray *e1, const ElementArray *e2) {
    int res = e1 && e2 && (e1->len == e2->len);
    int i = e1->len;
    while(res && i--) {
        res = res && element_equals(&e1->elements[i], &e2->elements[i]);
    }
    return res;
}


ElementArray *new_element_array(int initial_capacity) {
    int size = sizeof(ElementArray) + sizeof(Element) * initial_capacity;

    ElementArray *ar =  malloc(size);
    ar->len = 0;
    return ar;
}


ElementArray *new_element_array_from_fixed_array(int len, const Element elements[]) {
    ElementArray *ea = new_element_array(len);
    ea->len = len;
    memcpy(ea->elements, elements, sizeof(Element) * len);
    return ea;
}

void element_array_print(const ElementArray *ea) {
    element_array_print_with_indent(0, ea);
}


void element_array_print_with_indent(int indent, const ElementArray *ea) {
    int i;

    print_indent(indent);
    printf("len %d\n", ea->len);

    for(i = 0; i < ea->len; i++) {
        element_print_with_indent(indent, &ea->elements[i]);
    }
}


static void assert_element_array_equals(const ElementArray *expect_true, const ElementArray *expect_false, const ElementArray *input) {
    int actual;

    actual = element_array_equals(expect_true, input);
    assert(actual);

    actual = element_array_equals(input, expect_true);
    assert(actual);

    actual = element_array_equals(expect_false, input);
    assert(!actual);

    actual = element_array_equals(input, expect_false);
    assert(!actual);
}


#define NEW_ELEMENT_ARRAY(a) new_element_array_from_fixed_array(sizeof(a) / sizeof(Element), a)


static void test_element_array_equals_empty() {
    Element input_array[] = {};
    ElementArray *input = NEW_ELEMENT_ARRAY(input_array);

    Element expect_true_array[] = {};
    ElementArray *expect_true = NEW_ELEMENT_ARRAY(expect_true_array);

    Element expect_false_array[] = {{ELEMENT_NUMBER, .u.number = 1}};
    ElementArray *expect_false = NEW_ELEMENT_ARRAY(expect_false_array);

    assert_element_array_equals(expect_true, expect_false, input);
}

static void test_element_array_equals_length() {
    Element input_contents[] = {{ELEMENT_NUMBER, .u.number = 1}};
    ElementArray *input = NEW_ELEMENT_ARRAY(input_contents);

    Element expect_true_contents[] = {{ELEMENT_NUMBER, .u.number = 1}};
    ElementArray *expect_true = NEW_ELEMENT_ARRAY(expect_true_contents);

    Element expect_false_contents[] = {{ELEMENT_NUMBER, .u.number = 1}, {ELEMENT_NUMBER, .u.number = 1}};
    ElementArray *expect_false = NEW_ELEMENT_ARRAY(expect_false_contents);

    assert_element_array_equals(expect_true, expect_false, input);
}

static void test_element_array_equals_element() {
    Element input_contents[] = {{ELEMENT_LITERAL_NAME, .u.name = "abc"}};
    ElementArray *input = NEW_ELEMENT_ARRAY(input_contents);

    Element expect_true_contents[] = {{ELEMENT_LITERAL_NAME, .u.name = "abc"}};
    ElementArray *expect_true = NEW_ELEMENT_ARRAY(expect_true_contents);

    Element expect_false_contents[] = {{ELEMENT_EXECUTABLE_NAME, .u.name = "abc"}};
    ElementArray *expect_false = NEW_ELEMENT_ARRAY(expect_false_contents);

    assert_element_array_equals(expect_true, expect_false, input);
}

static void test_element_array_equals_nested() {
    Element input_contents[] = {{ELEMENT_NUMBER, .u.number = 1}};
    Element input_contents2[] = {{ELEMENT_EXEC_ARRAY, .u.exec_array = NEW_ELEMENT_ARRAY(input_contents)}};
    ElementArray *input = NEW_ELEMENT_ARRAY(input_contents2);

    Element expect_true_contents[] = {{ELEMENT_NUMBER, .u.number = 1}};
    Element expect_true_contents2[] = {{ELEMENT_EXEC_ARRAY, .u.exec_array = NEW_ELEMENT_ARRAY(expect_true_contents)}};
    ElementArray *expect_true = NEW_ELEMENT_ARRAY(expect_true_contents2);

    Element expect_false_contents[] = {{ELEMENT_NUMBER, .u.number = 2}};
    Element expect_false_contents2[] = {{ELEMENT_EXEC_ARRAY, .u.exec_array = NEW_ELEMENT_ARRAY(expect_false_contents)}};
    ElementArray *expect_false = NEW_ELEMENT_ARRAY(expect_false_contents2);

    assert_element_array_equals(expect_true, expect_false, input);
}


void element_array_test_all() {
    test_element_array_equals_empty();
    test_element_array_equals_length();
    test_element_array_equals_element();
    test_element_array_equals_nested();
}

