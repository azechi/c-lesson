#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "clesson.h"


#define NAME_SIZE 256


static char* malloc_copy_str(int size, char* src) {
    assert(size > 0);

    char* dest = (char*)malloc(size);

    do {
        dest[size] = src[size];

    } while (size--);

    return dest;
}


int parse_one(int prev_ch, struct Token *out_token) {
    int ch;

    if(prev_ch == EOF) {
        prev_ch = cl_getc();
    }

    ch = cl_getc();

    if (prev_ch == EOF) {
        out_token->ltype = LEX_END_OF_FILE;
    } else if ('0' <= prev_ch && prev_ch <= '9') {
        int acm = prev_ch - '0';

        while ('0' <= ch && ch <= '9') {
            acm = acm * 10 + ch - '0';
            ch = cl_getc();
        }

        out_token->ltype = LEX_NUMBER;
        out_token->u.number = acm;
    } else if (prev_ch == ' ') {

        while (ch == ' '){
            ch = cl_getc();
        }

        out_token->ltype = LEX_SPACE;
        out_token->u.onechar  = ' ';
    } else if ('a' <= prev_ch && prev_ch <= 'z') {
        char buf[NAME_SIZE];
        int i = 0;

        buf[i++] = prev_ch;

        while (('a' <= ch && ch <= 'z')
                || ('0' <= ch && ch <= '9')) {
            buf[i++] = ch;
            ch = cl_getc();
        }
        buf[i] = '\0';

        char* name = malloc_copy_str(i + 1, buf);
        out_token->ltype = LEX_EXECUTABLE_NAME;
        out_token->u.name = name;
    } else if (prev_ch == '/' && 'a' <= ch && ch <= 'z') {
        char buf[NAME_SIZE];
        int i = 0;

        buf[i++] = ch;
        ch = cl_getc();

        while (('a' <= ch && ch <= 'z')
                || ('0' <= ch && ch <= '9')) {
            buf[i++] = ch;
            ch = cl_getc();
        }
        buf[i] = '\0';

        char* name = malloc_copy_str(i + 1, buf);
        out_token->ltype = LEX_LITERAL_NAME;
        out_token->u.name = name;
    } else if (prev_ch == '{') {
        out_token->ltype = LEX_OPEN_CURLY;
        out_token->u.onechar = '{';
    } else if (prev_ch == '}') {
        out_token->ltype = LEX_CLOSE_CURLY;
        out_token->u.onechar = '}';
    } else {
        out_token->ltype = LEX_UNKNOWN;
    }

    return ch;
}


void parser_print_all() {
    int ch = EOF;
    struct Token token = {
        LEX_UNKNOWN,
        {0}
    };

    do {
        ch = parse_one(ch, &token);
        if(token.ltype != LEX_UNKNOWN) {
            switch(token.ltype) {
                case LEX_NUMBER:
                    printf("num: %d\n", token.u.number);
                    break;
                case LEX_SPACE:
                    printf("space!\n");
                    break;
                case LEX_OPEN_CURLY:
                    printf("Open curly brace '%c'\n", token.u.onechar);
                    break;
                case LEX_CLOSE_CURLY:
                    printf("Close curly brace '%c'\n", token.u.onechar);
                    break;
                case LEX_EXECUTABLE_NAME:
                    printf("EXECUTABLE_NAME: %s\n", token.u.name);
                    break;
                case LEX_LITERAL_NAME:
                    printf("LITERAL_NAME: %s\n", token.u.name);
                    break;

                default:
                    printf("Unknown type %d\n", token.ltype);
                    break;
            }
        }
    }while(ch != EOF);
}


static void test_parse_one_empty_should_return_END_OF_FILE() {
    char *input = "";
    int expect = LEX_END_OF_FILE;

    struct Token token = {LEX_UNKNOWN, {0}};
    int ch;

    cl_getc_set_src(input);
    ch = parse_one(EOF, &token);

    assert(ch == EOF);
    assert(token.ltype == expect);
}

static void test_parse_one_number() {
    char *input = "123";
    int expect = 123;

    struct Token token = {LEX_UNKNOWN, {0}};
    int ch;

    cl_getc_set_src(input);

    ch = parse_one(EOF, &token);

    assert(ch == EOF);
    assert(token.ltype == LEX_NUMBER);
    assert(expect == token.u.number);
}

static void test_malloc_copy_str_empty() {
    int input_size = 1;
    char* input_str = "";

    char* actual = malloc_copy_str(input_size, input_str);

    assert(actual != input_str);
    assert(strcmp(actual, input_str) == 0);
}

static void test_malloc_copy_str() {
    int input_size = 3;
    char* input_str = "ab";

    char* actual = malloc_copy_str(input_size, input_str);

    assert(actual != input_str);
    assert(strcmp(actual, input_str) == 0);
}

static void test_parse_one_executable_name() {
    char* input = "add";
    int expect_type = LEX_EXECUTABLE_NAME;
    char* expect_name = "add";

    struct Token actual = {LEX_UNKNOWN, {0}};
    int ch;

    cl_getc_set_src(input);

    ch = parse_one(EOF, &actual);

    assert(EOF == ch);
    assert(expect_type == actual.ltype);
    assert(strcmp(expect_name, actual.u.name) == 0);
}

static void test_parse_one_literal_name() {
    char* input = "/add";
    int expect_type = LEX_LITERAL_NAME;
    char* expect_name = "add";

    struct Token actual = {LEX_UNKNOWN, {0}};
    int ch;

    cl_getc_set_src(input);

    ch = parse_one(EOF, &actual);

    assert(EOF == ch);
    assert(expect_type == actual.ltype);
    assert(strcmp(expect_name, actual.u.name) == 0);
}

static void test_parse_one_open_curly() {
    char* input = "{";
    int expect_type = LEX_OPEN_CURLY;
    char expect_onechar = '{';

    struct Token actual = {LEX_UNKNOWN, {0}};
    int ch;

    cl_getc_set_src(input);

    ch = parse_one(EOF, &actual);

    assert(EOF == ch);
    assert(expect_type == actual.ltype);
    assert(expect_onechar == actual.u.onechar);
}

static void test_parse_one_close_curly() {
    char* input = "}";
    int expect_type = LEX_CLOSE_CURLY;
    char expect_onechar = '}';

    struct Token actual = {LEX_UNKNOWN, {0}};
    int ch;

    cl_getc_set_src(input);

    ch = parse_one(EOF, &actual);

    assert(EOF == ch);
    assert(expect_type == actual.ltype);
    assert(expect_onechar == actual.u.onechar);
}

__attribute__((unused))
static void unit_tests() {
    test_parse_one_empty_should_return_END_OF_FILE();
    test_parse_one_number();

    test_malloc_copy_str_empty();
    test_malloc_copy_str();

    test_parse_one_executable_name();
    test_parse_one_literal_name();
    test_parse_one_open_curly();
    test_parse_one_close_curly();
}

#if 0
int main() {
    unit_tests();

    cl_getc_set_src("123 45 add /some { 2 3 add } def");
    parser_print_all();
    return 1;
}
#endif
