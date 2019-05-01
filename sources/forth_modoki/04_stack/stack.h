#include <stdio.h>

enum LexicalType {
    NUMBER,
    LITERAL_NAME
};

struct Token {
    enum LexicalType ltype;
    union {
        int number;
        char *name;
    } u;
};

/*
    return NULL if stack empty and out_token is NULL
*/
struct Token *stack_pop();

void stack_push(const struct Token token);

