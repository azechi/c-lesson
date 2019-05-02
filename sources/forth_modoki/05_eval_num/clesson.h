#include <stdio.h>

enum LexicalType {
    LEX_NUMBER,
    LEX_SPACE,
    LEX_EXECUTABLE_NAME,
    LEX_LITERAL_NAME,
    LEX_OPEN_CURLY,
    LEX_CLOSE_CURLY,
    LEX_END_OF_FILE,
    LEX_UNKNOWN
};


struct Token {
    enum LexicalType ltype;
    union {
        int number;
        char onechar;
        char *name;
    } u;
};

enum ElementType {
    ELEMENT_NUMBER = LEX_NUMBER,
    ELEMENT_LITERAL_NAME = LEX_LITERAL_NAME
};

struct Element {
    enum ElementType etype;
    union {
        int number;
        char *name;
    } u;
};


/*
return one character and move cursor.
return EOF if end of file.
*/
int cl_getc();
void cl_getc_set_src(char* str);

int parse_one(int prev_ch, struct Token *out_token);


/*
    return NULL if stack empty
*/
struct Element *stack_pop();

void stack_push(const struct Element *token);

void stack_clear();
