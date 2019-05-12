#include <stdio.h>
#include <string.h>


#define assert_fail(msg) assert(0&&(msg))


int streq(const char *s1, const char *s2);

typedef enum LexicalType_ {
    LEX_UNKNOWN = 0,
    LEX_NUMBER,
    LEX_SPACE,
    LEX_EXECUTABLE_NAME,
    LEX_LITERAL_NAME,
    LEX_OPEN_CURLY,
    LEX_CLOSE_CURLY,
    LEX_END_OF_FILE
} LexicalType;


typedef struct Token_ {
    enum LexicalType_ ltype;
    union {
        int number;
        char onechar;
        char *name;
    } u;
} Token;

typedef enum ElementType_ {
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

int element_equals(const Element *e1, const Element *e2);

void element_print(const Element *el);

typedef struct ElementArray_ {
    int len;
    struct Element_ elements[0];
} ElementArray;

typedef struct AutoElementArray_ {
    int size;
    struct ElementArray_ *var_array;
} AutoElementArray;

ElementArray *new_element_array(int length, const Element *elements);

int element_array_equals(const ElementArray *e1, const ElementArray *e2);

void element_array_print(const ElementArray *ea);

void auto_element_array_init(int initial_size, AutoElementArray *out);
void auto_element_array_add_element(AutoElementArray * array, const Element *el);

/*
   return one character and move cursor.
   return EOF if end of file.
   */
int cl_getc();
void cl_getc_set_src(const char* str);

int parse_one(int prev_ch, Token *out_token);


/*
   return NULL if stack empty
   */
Element *try_stack_pop();

void stack_push(const Element *el);

void stack_clear();

void stack_print_all();

/*
   return 1 if key exists
   return 0 if key not exists
   */
int dict_get(const char* key, Element *out_el);

void dict_put(const char* key, const Element *el);

void dict_clear();

void dict_print_all();

