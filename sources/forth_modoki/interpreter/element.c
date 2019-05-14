#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "util.h"
#include "element.h"


int element_equals(const Element *e1, const Element *e2) {
    if(e1->etype == e2->etype) {
        switch(e1->etype) {
            case ELEMENT_NUMBER:
                return e1->u.number == e2->u.number;
            case ELEMENT_EXECUTABLE_NAME:
            case ELEMENT_LITERAL_NAME:
                return strcmp(e1->u.name, e2->u.name) == 0;
            case ELEMENT_EXEC_ARRAY:
                return element_array_equals(e1->u.exec_array, e2->u.exec_array);
            case ELEMENT_C_FUNC:
            default:
                assert_fail("NOT IMPLEMENTED");
                return 0;
        }
    }
    return 0;
}


void element_print(const Element *el) {
    element_print_with_indent(0, el);
}


void element_print_with_indent(int indent, const Element *el) {
    print_indent(indent);
    switch(el->etype) {
        case ELEMENT_NUMBER:
            printf("number %d\n", el->u.number);
            break;
        case ELEMENT_EXECUTABLE_NAME:
            printf("executable name %s\n", el->u.name);
            break;
        case ELEMENT_LITERAL_NAME:
            printf("literal name %s\n", el->u.name);
            break;
        case ELEMENT_C_FUNC:
            printf("c func %p\n", el->u.cfunc);
            break;
        case ELEMENT_EXEC_ARRAY:
            printf("exec array  len %d\n", el->u.exec_array->len);
            element_array_print_with_indent(indent + 4, el->u.exec_array);
            break;
        default:
            printf("unknown %d", el->etype);
            break;
    }
}


static void assert_element_equals(const Element *expect_true, const Element *expect_false, const Element *input);
#define NEW_ELEMENT_ARRAY(a) new_element_array_from_fixed_array(sizeof(a) / sizeof(Element), a)


static void test_element_equals_name() {
    Element input = {ELEMENT_LITERAL_NAME, .u.name = "abc"};
    Element expect_true = {ELEMENT_LITERAL_NAME, .u.name = "abc"};
    Element expect_false = {ELEMENT_EXECUTABLE_NAME, .u.name = "abc"};

    assert_element_equals(&expect_true, &expect_false, &input);
}

static void test_element_equals_number() {
    Element input = {ELEMENT_NUMBER, .u.number = 1};
    Element expect_true = {ELEMENT_NUMBER, .u.number = 1};
    Element expect_false = {ELEMENT_NUMBER, .u.number = 2};

    assert_element_equals(&expect_true, &expect_false, &input);
}

static void test_element_equals_exec_array() {
    Element input_array[] = {{ELEMENT_NUMBER, .u.number = 1}, {ELEMENT_NUMBER, .u.number = 2}};
    Element input = {ELEMENT_EXEC_ARRAY, .u.exec_array = NEW_ELEMENT_ARRAY(input_array)};

    Element expect_true_array[] = {{ELEMENT_NUMBER, .u.number = 1}, {ELEMENT_NUMBER, .u.number = 2}};
    Element expect_true = {ELEMENT_EXEC_ARRAY, .u.exec_array = NEW_ELEMENT_ARRAY(expect_true_array)};

    Element expect_false_array[] = {{ELEMENT_NUMBER, .u.number = 1}, {ELEMENT_NUMBER, .u.number = 3}};
    Element expect_false = {ELEMENT_EXEC_ARRAY, .u.exec_array = NEW_ELEMENT_ARRAY(expect_false_array)};

    assert_element_equals(&expect_true, &expect_false, &input);
}


void element_test_all() {
    test_element_equals_name();
    test_element_equals_number();
    test_element_equals_exec_array();

}


static void assert_element_equals(const Element *expect_true, const Element *expect_false, const Element *input) {
    int actual;

    actual = element_equals(expect_true, input);
    assert(actual);

    actual = element_equals(input, expect_true);
    assert(actual);

    actual = element_equals(expect_false, input);
    assert(!actual);

    actual = element_equals(input, expect_false);
    assert(!actual);
}

