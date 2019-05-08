#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "clesson.h"


int element_equals(const struct Element e1, const struct Element e2) {
    if(e1.etype == e2.etype) {
        switch(e1.etype) {
            case ELEMENT_NUMBER:
                return (e1.u.number == e2.u.number);
            case ELEMENT_EXECUTABLE_NAME:
            case ELEMENT_LITERAL_NAME:
                return (strcmp(e1.u.name, e2.u.name) == 0);
            case ELEMENT_EXEC_ARRAY:
                return (element_array_equals(e1.u.exec_array, e1.u.exec_array));
            case ELEMENT_C_FUNC:
            default:
                assert_fail("NOT IMPLEMENTED");
                return 0;
        }
    }
    return 0;
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

int element_array_equals(const struct ElementArray *e1, const struct ElementArray *e2) {
    if (e1 && e2 && e1->len == e2->len){
        int i;
        for(i = 0; i < e1->len; i++) {
           if(!element_equals(e1->elements[i], e2->elements[i])) {
               return 0;
           }
        }
        return 1;
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

void element_array_print(const struct ElementArray ea) {
    printf("len %d\n", ea.len);
    element_array_print_with_indent(0, &ea);
}

__attribute__((unused))
static void test_element_equals() {
    struct Element input_number_0 = {ELEMENT_NUMBER, {0}};
    struct Element input_number_0_0 = {ELEMENT_NUMBER, {0}};
    struct Element input_number_1 = {ELEMENT_NUMBER, {1}};
    struct Element input_literal_name_0 = {ELEMENT_LITERAL_NAME, .u.name = "a"};
    struct Element input_literal_name_0_0 = {ELEMENT_LITERAL_NAME, .u.name = "a"};
    struct Element input_literal_name_1 = {ELEMENT_LITERAL_NAME, .u.name = "b"};

    assert(element_equals(input_number_0, input_number_0_0));
    assert(!element_equals(input_number_0, input_number_1));

    assert(element_equals(input_literal_name_0, input_literal_name_0_0));
    assert(!element_equals(input_literal_name_0, input_literal_name_1));
}

#if 0
int main() { 
    test_element_equals();

    return 0;
}
#endif
