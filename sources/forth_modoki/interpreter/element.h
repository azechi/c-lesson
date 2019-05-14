#ifndef ELEMENT_H
#define ELEMENT_H


typedef enum ElementType_ {
    ELEMENT_UNKNOWN = 0,
    ELEMENT_NUMBER,
    ELEMENT_EXECUTABLE_NAME,
    ELEMENT_LITERAL_NAME,
    ELEMENT_C_FUNC,
    ELEMENT_EXEC_ARRAY
} ElementType;


typedef struct Element_ {
    enum ElementType_ etype;
    union {
        int number;
        char *name;
        void (*cfunc)();
        struct ElementArray_ *exec_array;
    } u;
} Element;


typedef struct ElementArray_ {
    int len;
    struct Element_ elements[0];
} ElementArray;


int element_equals(const Element *e1, const Element *e2);

int element_array_equals(const ElementArray *e1, const ElementArray *e2);


ElementArray *new_element_array(int initial_capacity);

ElementArray *new_element_array_from_fixed_array(int len, const Element elements[]);


void element_print(const Element *el);

void element_print_with_indent(int indent, const Element *el);

void element_array_print(const ElementArray *ea);

void element_array_print_with_indent(int indent, const ElementArray *el);


void element_test_all();

void element_array_test_all();


#endif
