#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "util.h"
#include "parser.h"

#define NAME_SIZE 256


static char* malloc_copy_str(int size, char* src) {
    assert(size > 0);

    char* dest = (char*)malloc(size);

    do {
        dest[size] = src[size];

    } while (size--);

    return dest;
}

static int is_digit(char ch) {
    return '0' <= ch && ch <= '9';
}

static int is_alpha(char ch) {
    return ('a' <= ch && ch <= 'z')
        || ('A' <= ch && ch <= 'Z');
}

static int is_alpha_digit(char ch) {
    return is_digit(ch) || is_alpha(ch);
}


static int token_equals(const Token *t1, const Token *t2) {
    if(!(t1 && t2
                && t1->ltype == t2->ltype)){
        return 0;
    }

    switch(t1->ltype) {
        case LEX_NUMBER:
            return (t1->u.number == t2->u.number);
        case LEX_LITERAL_NAME:
        case LEX_EXECUTABLE_NAME:
            return streq(t1->u.name, t2->u.name);
        case LEX_OPEN_CURLY:
        case LEX_CLOSE_CURLY:
        case LEX_SPACE:
            return (t1->u.onechar == t2->u.onechar);
        case LEX_END_OF_FILE:
            return 1;
        case LEX_UNKNOWN:
            return 0;
    }
}

static void verify_parse_one(const char *input, const Token *expect, int expect_ch) {
    cl_getc_set_src(input);

    Token actual = {0};
    int actual_ch = parse_one(EOF, &actual);
    int eq = token_equals(expect, &actual);
    assert(eq && expect_ch ==  actual_ch);
}

static void verify_parse_one_number(const char *input, int expect_number, int expect_ch) {
    Token expect = {LEX_NUMBER, .u.number = expect_number};
    verify_parse_one(input, &expect, expect_ch);
}

static void test_parse_one_number() {
    verify_parse_one_number("123", 123, EOF);
    verify_parse_one_number("-123", -123, EOF);
    verify_parse_one_number("1{", 1, '{');
}

static int parse_one_number(int prev_ch, int ch, Token *out_token) {
    int is_positive = prev_ch != '-';
    int acm = (is_positive) ? prev_ch - '0' : 0;

    while (is_digit(ch)) {
        acm = acm * 10 + ch - '0';
        ch = cl_getc();
    }

    out_token->ltype = LEX_NUMBER;
    out_token->u.number = (is_positive) ? acm : -acm;

    return ch;
}


static void verify_parse_one_executable_name(const char *input, const char *expect_name, int expect_ch) {
    Token expect = {LEX_EXECUTABLE_NAME, .u.name = (char*)expect_name};
    verify_parse_one(input, &expect, expect_ch);
}

static void verify_parse_one_literal_name(const char *input, const char *expect_name, int expect_ch) {
    Token expect = {LEX_LITERAL_NAME, .u.name = (char*)expect_name};
    verify_parse_one(input, &expect, expect_ch);
}

static void test_parse_one_executable_name() {
    verify_parse_one_executable_name("ab", "ab", EOF);
    verify_parse_one_executable_name("a123", "a123", EOF);
    verify_parse_one_executable_name("a123{", "a123", '{');
}

static void test_parse_one_literal_name() {
    verify_parse_one_literal_name("/ab", "ab", EOF);
    verify_parse_one_literal_name("/a123", "a123", EOF);
    verify_parse_one_literal_name("/a123{", "a123", '{');
}

static int parse_one_name(int prev_ch, int ch, Token *out_token) {
    int is_exec_name = prev_ch != '/';

    int i = 0;
    char buf[NAME_SIZE];

    if (is_exec_name) {
        buf[i++] = prev_ch;
    } else {
        buf[i++] = ch;
        ch = cl_getc();
    }

    while (is_alpha_digit(ch) || ch == '_') {
        buf[i++] = ch;
        ch = cl_getc();
    }
    buf[i] = '\0';

    char* name = malloc_copy_str(i + 1, buf);
    out_token->ltype = (is_exec_name) ? LEX_EXECUTABLE_NAME : LEX_LITERAL_NAME;
    out_token->u.name = name;

    return ch;
}


static void verify_parse_one_space(const char *input, int expect_ch) {
    Token expect = {LEX_SPACE, .u.onechar = ' '};
    verify_parse_one(input, &expect, expect_ch);
}

static void test_parse_one_space() {
    verify_parse_one_space(" ", EOF);
    verify_parse_one_space("  ", EOF);
    verify_parse_one_space("  /", '/');
}

static void test_parse_one_LF() {
    verify_parse_one_space("\n ", EOF);
    verify_parse_one_space(" \n  \n ", EOF);
    verify_parse_one_space("  \n  \n-1 ", '-');
}

static void test_parse_one_comment() {
    verify_parse_one_space("% 123 \n0", '0');
    verify_parse_one_space("%123 \n0", '0');
    verify_parse_one_space("% % \n0", '0');
    verify_parse_one_space("% % ", EOF);
}

int parse_one(int prev_ch, Token *out_token) {
    int ch;

    if(prev_ch == EOF) {
        prev_ch = cl_getc();
    }

    ch = cl_getc();

    if (prev_ch == EOF) {
        out_token->ltype = LEX_END_OF_FILE;
    } else if (is_digit(prev_ch)
                || (prev_ch == '-' && is_digit(ch))) {
        ch = parse_one_number(prev_ch, ch, out_token);
    } else if (is_alpha(prev_ch)
            || (prev_ch == '/' && is_alpha(ch)))
    {
        ch = parse_one_name(prev_ch, ch, out_token);
    } else if (prev_ch == '%') {
        while (ch != '\n' && ch != EOF) {
            ch = cl_getc();
        }

        ch = cl_getc();
        out_token->ltype = LEX_SPACE;
        out_token->u.onechar = ' ';
    }  else if (prev_ch == ' '
            || prev_ch == '\n') {

        while (ch == ' '
                || ch == '\n'){
            ch = cl_getc();
        }

        out_token->ltype = LEX_SPACE;
        out_token->u.onechar  = ' ';
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



static void test_parse_one_empty_should_return_END_OF_FILE() {
    char *input = "";
    int expect = LEX_END_OF_FILE;

    Token token = {LEX_UNKNOWN, {0}};
    int ch;

    cl_getc_set_src(input);
    ch = parse_one(EOF, &token);

    assert(ch == EOF);
    assert(token.ltype == expect);
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


static void test_parse_one_open_curly() {
    char* input = "{";
    int expect_type = LEX_OPEN_CURLY;
    char expect_onechar = '{';

    Token actual = {LEX_UNKNOWN, {0}};
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

    Token actual = {LEX_UNKNOWN, {0}};
    int ch;

    cl_getc_set_src(input);

    ch = parse_one(EOF, &actual);

    assert(EOF == ch);
    assert(expect_type == actual.ltype);
    assert(expect_onechar == actual.u.onechar);
}

void parser_test_all() {
    test_parse_one_empty_should_return_END_OF_FILE();
    test_parse_one_number();

    test_malloc_copy_str_empty();
    test_malloc_copy_str();

    test_parse_one_executable_name();
    test_parse_one_literal_name();
    test_parse_one_open_curly();
    test_parse_one_close_curly();

    test_parse_one_space();
    test_parse_one_LF();
    test_parse_one_comment();
}


void parser_print_all() {
    int ch = EOF;
    Token token = {0};

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

