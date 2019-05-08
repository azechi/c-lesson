#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "clesson.h"


int element_array_equals(const struct ElementArray *e1, const struct ElementArray *e2) {
    int res = e1 && e2 && (e1->len == e2->len);
    int i = e1->len;
    while(res && i--) {
        res = res && element_equals(e1->elements[i], e2->elements[i]);
    }
    return res;
}

int element_equals(const struct Element e1, const struct Element e2) {
    if(e1.etype == e2.etype) {
        switch(e1.etype) {
            case ELEMENT_NUMBER:
                return (e1.u.number == e2.u.number);
            case ELEMENT_EXECUTABLE_NAME:
            case ELEMENT_LITERAL_NAME:
                return (strcmp(e1.u.name, e2.u.name) == 0);
            case ELEMENT_EXEC_ARRAY:
                return (element_array_equals(e1.u.exec_array, e2.u.exec_array));
            case ELEMENT_C_FUNC:
            default:
                assert_fail("NOT IMPLEMENTED");
                return 0;
        }
    }
    return 0;
}

struct ElementArray *new_element_array(int length, struct Element *elements) {
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
        element_print(ea->elements[i]);
    }
}

static void element_print_with_indent(int indent, const struct Element el) {
    print_indent(indent);
    switch(el.etype) {
        case ELEMENT_NUMBER:
            printf("number %d\n", el.u.number);
            break;
        case ELEMENT_EXECUTABLE_NAME:
            printf("executable name %s\n", el.u.name);
            break;
        case ELEMENT_LITERAL_NAME:
            printf("literal name %s\n", el.u.name);
            break;
        case ELEMENT_C_FUNC:
            printf("c func %p\n", el.u.cfunc);
            break;
        case ELEMENT_EXEC_ARRAY:
            printf("exec array  len %d\n", el.u.exec_array->len);
            element_array_print_with_indent(indent + 4, el.u.exec_array);
            break;
        default: 
            printf("unknown %d", el.etype);
            break;
    }
}

void element_print(const struct Element el) {
    element_print_with_indent(0, el);
}


void element_array_print(const struct ElementArray ea) {
    printf("len %d\n", ea.len);
    element_array_print_with_indent(0, &ea);
}

struct Element element_number(int i) {
    return (struct Element){ELEMENT_NUMBER, .u.number = i};
}

struct Element element_literal_name(char *s) {
    return (struct Element){ELEMENT_LITERAL_NAME, .u.name = s};
}

struct Element element_execution_name(char *s) {
    return (struct Element){ELEMENT_EXECUTABLE_NAME, .u.name = s};
}

struct Element element_exec_array(struct ElementArray *ea) {
    return (struct Element){ELEMENT_EXEC_ARRAY, .u.exec_array = ea};
}


static void assert_element_equals(struct Element expect_true, struct Element expect_false, struct Element input) {
    assert(element_equals(expect_true, input));
    assert(element_equals(input, expect_true));

    assert(!element_equals(expect_false, input));
    assert(!element_equals(input, expect_false));
}

static void assert_element_array_equals(struct ElementArray *expect_true, struct ElementArray *expect_false, struct ElementArray *input) {

    assert(element_array_equals(expect_true, input));
    assert(element_array_equals(input, expect_true));

    assert(!element_array_equals(expect_false, input));
    assert(!element_array_equals(expect_false, input));
}


#define EA(a) new_element_array(sizeof(a) / sizeof(struct Element), a)


static void test_element_equals_name() {

    struct Element input = element_literal_name("abc");
    struct Element expect_true = element_literal_name("abc");
    struct Element expect_false = element_execution_name("abc");

    assert_element_equals(expect_true, expect_false, input);
}

static void test_element_equals_number() {
    struct Element input = element_number(1);
    struct Element expect_true = element_number(1);
    struct Element expect_false = element_number(2);

    assert_element_equals(expect_true, expect_false, input);
}

static void test_element_equals_exec_array() {
    struct Element input_array[] = {element_number(1), element_number(2)};
    struct Element input = element_exec_array(EA(input_array));

    struct Element expect_true_array[] = {element_number(1), element_number(2)};
    struct Element expect_true = element_exec_array(EA(expect_true_array));

    struct Element expect_false_array[] = {element_number(1), element_number(3)};
    struct Element expect_false = element_exec_array(EA(expect_false_array));

    assert_element_equals(expect_true, expect_false, input);
}


static void test_element_array_equals_empty() {
    struct Element input[] = {};
    struct Element expect_true[] = {};
    struct Element expect_false[] = {element_number(1)};

    assert_element_array_equals(EA(expect_true), EA(expect_false), EA(input));
}


static void test_element_array_equals_length() {
    struct Element input[] = {element_number(1)};
    struct Element expect_true[] = {element_number(1)};
    struct Element expect_false[] = {element_number(1), element_number(1)};

    assert_element_array_equals(EA(expect_true), EA(expect_false), EA(input));
}

static void test_element_array_equals_element() {
    struct Element input[] = {element_literal_name("abc")};
    struct Element expect_true[] = {element_literal_name("abc")};
    struct Element expect_false[] = {element_execution_name("abc")};

    assert_element_array_equals(EA(expect_true), EA(expect_false), EA(input));
}

static void test_element_array_equals_nested() {
    struct Element input_children[] = {element_number(1)};
    struct Element input[] = {element_exec_array(EA(input_children))};

    struct Element expect_true_children[] = {element_number(1)};
    struct Element expect_true[] = {element_exec_array(EA(expect_true_children))};

    struct Element expect_false_children[] = {element_number(2)};
    struct Element expect_fales[] = {element_exec_array(EA(expect_false_children))};

    assert_element_array_equals(EA(expect_true), EA(expect_fales), EA(input));
}

#undef EA

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
