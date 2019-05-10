#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "clesson.h"


int element_array_equals(const struct ElementArray *e1, const struct ElementArray *e2) {
    int res = e1 && e2 && (e1->len == e2->len);
    int i = e1->len;
    while(res && i--) {
        res = res && element_equals(&e1->elements[i], &e2->elements[i]);
    }
    return res;
}

int element_equals(const struct Element *e1, const struct Element *e2) {
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

struct ElementArray *new_element_array(int length, const struct Element *elements) {
    int size = sizeof(struct ElementArray) + (sizeof(struct Element) * length);
    struct ElementArray *ea = malloc(size);

    ea->len = length;
    memcpy(ea->elements, elements, sizeof(struct Element) * length);
    return ea;
}


static void print_indent(int i) {
    char str[5]; /* "%99c"" */
    sprintf(str, "%%%dc", i % 100); 
    printf(str, ' ');
}

static void element_array_print_with_indent(int indent, const struct ElementArray *ea) {
    int i;

    print_indent(indent);
    for(i = 0; i < ea->len; i++) {
        element_print(&ea->elements[i]);
    }
}

static void element_print_with_indent(int indent, const struct Element *el) {
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

void element_print(const struct Element *el) {
    element_print_with_indent(0, el);
}

void element_array_print(const struct ElementArray *ea) {
    printf("len %d\n", ea->len);
    element_array_print_with_indent(0, ea);
}


static void assert_element_equals(const struct Element *expect_true, const struct Element *expect_false, const struct Element *input) {
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

static void assert_element_array_equals(const struct ElementArray *expect_true, const struct ElementArray *expect_false, const struct ElementArray *input) {
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


#define NEW_ELEMENT_ARRAY(a) new_element_array(sizeof(a) / sizeof(struct Element), a)


static void test_element_equals_name() {
    struct Element input = {ELEMENT_LITERAL_NAME, .u.name = "abc"};
    struct Element expect_true = {ELEMENT_LITERAL_NAME, .u.name = "abc"};
    struct Element expect_false = {ELEMENT_EXECUTABLE_NAME, .u.name = "abc"};

    assert_element_equals(&expect_true, &expect_false, &input);
}


static void test_element_equals_number() {
    struct Element input = {ELEMENT_NUMBER, .u.number = 1};
    struct Element expect_true = {ELEMENT_NUMBER, .u.number = 1};
    struct Element expect_false = {ELEMENT_NUMBER, .u.number = 2};

    assert_element_equals(&expect_true, &expect_false, &input);
}


static void test_element_equals_exec_array() {
    struct Element input_array[] = {{ELEMENT_NUMBER, .u.number = 1}, {ELEMENT_NUMBER, .u.number = 2}};
    struct Element input = {ELEMENT_EXEC_ARRAY, .u.exec_array = NEW_ELEMENT_ARRAY(input_array)};

    struct Element expect_true_array[] = {{ELEMENT_NUMBER, .u.number = 1}, {ELEMENT_NUMBER, .u.number = 2}};
    struct Element expect_true = {ELEMENT_EXEC_ARRAY, .u.exec_array = NEW_ELEMENT_ARRAY(expect_true_array)};

    struct Element expect_false_array[] = {{ELEMENT_NUMBER, .u.number = 1}, {ELEMENT_NUMBER, .u.number = 3}};
    struct Element expect_false = {ELEMENT_EXEC_ARRAY, .u.exec_array = NEW_ELEMENT_ARRAY(expect_false_array)};

    assert_element_equals(&expect_true, &expect_false, &input);
}


static void test_element_array_equals_empty() {
    struct Element input_array[] = {};
    struct ElementArray *input = NEW_ELEMENT_ARRAY(input_array);

    struct Element expect_true_array[] = {};
    struct ElementArray *expect_true = NEW_ELEMENT_ARRAY(expect_true_array);

    struct Element expect_false_array[] = {{ELEMENT_NUMBER, .u.number = 1}};
    struct ElementArray *expect_false = NEW_ELEMENT_ARRAY(expect_false_array);

    assert_element_array_equals(expect_true, expect_false, input);
}

static void test_element_array_equals_length() {
    struct Element input_contents[] = {{ELEMENT_NUMBER, .u.number = 1}};
    struct ElementArray *input = NEW_ELEMENT_ARRAY(input_contents);

    struct Element expect_true_contents[] = {{ELEMENT_NUMBER, .u.number = 1}};
    struct ElementArray *expect_true = NEW_ELEMENT_ARRAY(expect_true_contents);

    struct Element expect_false_contents[] = {{ELEMENT_NUMBER, .u.number = 1}, {ELEMENT_NUMBER, .u.number = 1}};
    struct ElementArray *expect_false = NEW_ELEMENT_ARRAY(expect_false_contents);

    assert_element_array_equals(expect_true, expect_false, input);
}

static void test_element_array_equals_element() {
    struct Element input_contents[] = {{ELEMENT_LITERAL_NAME, .u.name = "abc"}};
    struct ElementArray *input = NEW_ELEMENT_ARRAY(input_contents);

    struct Element expect_true_contents[] = {{ELEMENT_LITERAL_NAME, .u.name = "abc"}};
    struct ElementArray *expect_true = NEW_ELEMENT_ARRAY(expect_true_contents);

    struct Element expect_false_contents[] = {{ELEMENT_EXECUTABLE_NAME, .u.name = "abc"}};
    struct ElementArray *expect_false = NEW_ELEMENT_ARRAY(expect_false_contents);

    assert_element_array_equals(expect_true, expect_false, input);
}

static void test_element_array_equals_nested() {
    struct Element input_contents[] = {{ELEMENT_NUMBER, .u.number = 1}};
    struct Element input_contents2[] = {{ELEMENT_EXEC_ARRAY, .u.exec_array = NEW_ELEMENT_ARRAY(input_contents)}};
    struct ElementArray *input = NEW_ELEMENT_ARRAY(input_contents2);

    struct Element expect_true_contents[] = {{ELEMENT_NUMBER, .u.number = 1}};
    struct Element expect_true_contents2[] = {{ELEMENT_EXEC_ARRAY, .u.exec_array = NEW_ELEMENT_ARRAY(expect_true_contents)}};
    struct ElementArray *expect_true = NEW_ELEMENT_ARRAY(expect_true_contents2);

    struct Element expect_false_contents[] = {{ELEMENT_NUMBER, .u.number = 2}};
    struct Element expect_false_contents2[] = {{ELEMENT_EXEC_ARRAY, .u.exec_array = NEW_ELEMENT_ARRAY(expect_false_contents)}};
    struct ElementArray *expect_false = NEW_ELEMENT_ARRAY(expect_false_contents2);

    assert_element_array_equals(expect_true, expect_false, input);
}

#undef NEW_ELEMENT_ARRAY

__attribute__((unused))
    static void test_all() {
        test_element_equals_name();
        test_element_equals_number();
        test_element_equals_exec_array();

        test_element_array_equals_empty();
        test_element_array_equals_length();
        test_element_array_equals_element();
        test_element_array_equals_nested();
    }

#if 0
int main() { 
    test_all();

    return 0;
}
#endif
