#include <stdio.h>
#include <string.h>


#define assert_fail(msg) assert(0&&(msg))


int streq(char *s1, char *s2);

enum LexicalType {
    LEX_NUMBER,
    LEX_SPACE,
    LEX_EXECUTABLE_NAME,
    LEX_LITERAL_NAME,
    LEX_OPEN_CURLY,
    LEX_CLOSE_CURLY,
    LEX_END_OF_FILE,
    LEX_UNKNOWN,

    LEX_MAX = LEX_UNKNOWN
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
    ELEMENT_EXECUTABLE_NAME = LEX_EXECUTABLE_NAME,
    ELEMENT_LITERAL_NAME = LEX_LITERAL_NAME,

    ELEMENT_C_FUNC = LEX_MAX + 1,
    ELEMENT_EXEC_ARRAY
};

struct Element {
    enum ElementType etype;
    union {
        int number;
        char *name;
        void (*cfunc)();
        struct ElementArray *exec_array;
    } u;
};

struct Element element_number(int i);
struct Element element_executable_name(char *s);
struct Element element_literal_name(char *s);
struct Element element_exec_array(struct ElementArray *ea);
struct Element element_c_func(void (*f)());

int element_equals(const struct Element e1, const struct Element e2);

void element_print(const struct Element el);

struct ElementArray {
    int len;
    struct Element elements[0];
};

struct ElementArray *new_element_array(int length, struct Element *elements);

int element_array_equals(const struct ElementArray *e1, const struct ElementArray *e2);

void element_array_print(const struct ElementArray *ea);

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

void stack_push(const struct Element el);

void stack_clear();

void stack_print_all();

/*
   return 1 if key exists
   return 0 if key not exists
   */
int dict_get(char* key, struct Element *out_el);

void dict_put(char* key, struct Element el);

void dict_clear();

void dict_print_all();

