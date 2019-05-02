#include <stdio.h>

enum ElementType {
    ELEMENT_NUMBER,
    ELEMENT_LITERAL_NAME
};

struct Element {
    enum ElementType ltype;
    union {
        int number;
        char *name;
    } u;
};

/*
    return NULL if stack empty
*/
struct Element *stack_pop();


void stack_push(const struct Element *token);

