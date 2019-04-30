#include "clesson.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

enum LexicalType {
    NUMBER,
    SPACE,
    EXECUTABLE_NAME,
    LITERAL_NAME,
    OPEN_CURLY,
    CLOSE_CURLY, 
    END_OF_FILE,
    UNKNOWN
};



struct Token {
    enum LexicalType ltype;
    union {
        int number;
        char onechar;
        char *name;
    } u;
};

#define NAME_SIZE 256


char* malloc_copy_str(int len, char* src) {
    assert(len > 0);

    char* dest = (char*)malloc((len + sizeof(size_t)) / sizeof(size_t));

    do {
        dest[len] = src[len];

    } while (len--);

    return dest;
}


int parse_one(int prev_ch, struct Token *out_token) {
    int ch;

    if(prev_ch == EOF) {
        prev_ch = cl_getc();
    }

    ch = cl_getc();

    if (prev_ch == EOF) {
        out_token->ltype = END_OF_FILE;
    } else if ('0' <= prev_ch && prev_ch <= '9') {
        int acm = prev_ch - '0';
        
        while ('0' <= ch && ch <= '9') {
            acm = acm * 10 + ch - '0';
            ch = cl_getc();
        }
        
        out_token->ltype = NUMBER;
        out_token->u.number = acm;
    } else if (prev_ch == ' ') {
        
        while (ch == ' '){
            ch = cl_getc();
        }

        out_token->ltype = SPACE;
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
        out_token->ltype = EXECUTABLE_NAME;
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
        out_token->ltype = LITERAL_NAME;
        out_token->u.name = name;
    } else if (prev_ch == '{') {
        out_token->ltype = OPEN_CURLY;
        out_token->u.onechar = '{';
    } else if (prev_ch == '}') {
        out_token->ltype = CLOSE_CURLY;
        out_token->u.onechar = '}';
    } else {
        out_token->ltype = UNKNOWN;
    }

    return ch;
}


void parser_print_all() {
    int ch = EOF;
    struct Token token = {
        UNKNOWN,
        {0}
    };

    do {
        ch = parse_one(ch, &token);
        if(token.ltype != UNKNOWN) {
            switch(token.ltype) {
                case NUMBER:
                    printf("num: %d\n", token.u.number);
                    break;
                case SPACE:
                    printf("space!\n");
                    break;
                case OPEN_CURLY:
                    printf("Open curly brace '%c'\n", token.u.onechar);
                    break;
                case CLOSE_CURLY:
                    printf("Close curly brace '%c'\n", token.u.onechar);
                    break;
                case EXECUTABLE_NAME:
                    printf("EXECUTABLE_NAME: %s\n", token.u.name);
                    break;
                case LITERAL_NAME:
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
    int expect = END_OF_FILE;

    struct Token token = {UNKNOWN, {0}};
    int ch;

    cl_getc_set_src(input);
    ch = parse_one(EOF, &token);

    assert(ch == EOF);
    assert(token.ltype == expect);
}

static void test_parse_one_number() {
    char *input = "123";
    int expect = 123;

    struct Token token = {UNKNOWN, {0}};
    int ch;

    cl_getc_set_src(input);

    ch = parse_one(EOF, &token);

    assert(ch == EOF);
    assert(token.ltype == NUMBER);
    assert(expect == token.u.number);
}


static void test_parse_one_executable_name() {
    char* input = "add";
    int expect_type = EXECUTABLE_NAME;
    char* expect_name = "add";

    struct Token actual = {UNKNOWN, {0}};
    int ch;

    cl_getc_set_src(input);

    ch = parse_one(EOF, &actual);

    assert(EOF == ch);
    assert(expect_type == actual.ltype);
    assert(strcmp(expect_name, actual.u.name) == 0);
}

static void test_parse_one_literal_name() {
    char* input = "/add";
    int expect_type = LITERAL_NAME;
    char* expect_name = "add";

    struct Token actual = {UNKNOWN, {0}};
    int ch;

    cl_getc_set_src(input);

    ch = parse_one(EOF, &actual);

    assert(EOF == ch);
    assert(expect_type == actual.ltype);
    assert(strcmp(expect_name, actual.u.name) == 0);
}

static void test_parse_one_open_curly() {
    char* input = "{";
    int expect_type = OPEN_CURLY;
    char expect_onechar = '{';

    struct Token actual = {UNKNOWN, {0}};
    int ch;

    cl_getc_set_src(input);

    ch = parse_one(EOF, &actual);

    assert(EOF == ch);
    assert(expect_type == actual.ltype);
    assert(expect_onechar == actual.u.onechar);
}

static void test_parse_one_close_curly() {
    char* input = "}";
    int expect_type = CLOSE_CURLY;
    char expect_onechar = '}';

    struct Token actual = {UNKNOWN, {0}};
    int ch;

    cl_getc_set_src(input);

    ch = parse_one(EOF, &actual);

    assert(EOF == ch);
    assert(expect_type == actual.ltype);
    assert(expect_onechar == actual.u.onechar);
}


static void unit_tests() {
    test_parse_one_empty_should_return_END_OF_FILE();
    test_parse_one_number();
    test_parse_one_executable_name();
    test_parse_one_literal_name();
    test_parse_one_open_curly();
    test_parse_one_close_curly();
}

int main() {
    unit_tests();

    cl_getc_set_src("123 45 add /some { 2 3 add } def");
    //parser_print_all();
    return 1;
}
