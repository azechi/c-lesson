#ifndef ELEMENT_H
#define ELEMENT_H


/* auto_element_array.h */
struct AutoElementArray_;

typedef void (*CompileFunc)(struct AutoElementArray_ *);


typedef enum ElementType_ {
    ELEMENT_UNKNOWN = 0,
    ELEMENT_NUMBER,
    ELEMENT_EXECUTABLE_NAME,
    ELEMENT_LITERAL_NAME,
    ELEMENT_C_FUNC,
    ELEMENT_EXEC_ARRAY,
    ELEMENT_COMPILE_FUNC,
    ELEMENT_PRIMITIVE
} ElementType;


typedef enum Operator_ {
    OP_EXEC,
    OP_JMP,
    OP_JMP_NOT_IF,
    OP_STORE,
    OP_LOAD,
    OP_LPOP
} Operator;


typedef struct Element_ {
    enum ElementType_ etype;
    union {
        int number;
        char *name;
        void (*c_func)();
        struct ElementArray_ *exec_array;
        void (*compile_func)(struct AutoElementArray_ *);
        enum Operator_ operator;
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
