#include <stdio.h>
#include <string.h>

int streq(char *s1, char *s2);

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


int element_equals(const struct Element e1, const struct Element e2);


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
struct Element *try_stack_pop();

void stack_push(const struct Element *el);

void stack_clear();

/*
return 1 if key exists
return 0 if key not exists
*/
int dict_get(char* key, struct Element *out_el);

void dict_put(char* key, struct Element *el);

void dict_clear();

void dict_print_all();

