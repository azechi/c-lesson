#include <assert.h>
#include <ctype.h>
#include <string.h>
#include "parser.h"

typedef int (*Predicate_char)(int c);
static int skip(const char *s, Predicate_char pred);
static int is_space(int c);
static int is_one_first(int c);
static int is_one_rest(int c);


int str_eq_subs(const char *s, const Substring *subs) {
    int len = strlen(s);
    return len == subs->len && strncmp(s, subs->str, len) == 0;
}

int parse_one(const char *s, Substring *out_subs) {
    int i = skip(s, is_space);
    char c = s[i];

    if(c == '\0') {
        return PARSE_EOF;
    }

    if(is_one_first(c)) {
        const char *subs = s + i;
        int len = skip(subs, is_one_rest);
        out_subs->str = subs;
        out_subs->len = len;
        return i + len;
    }

    return PARSE_FAILURE;
}

int parse_register(const char *s, int *out_register) {
    int i = skip(s, is_space);
    char c = s[i];

    if(!is_register(s)) {
        return PARSE_FAILURE;
    }
    c = s[++i];

    if(isdigit(c)) {
        int len = skip(s + i, isdigit);
        int n = 0;
        for(; len > 0; len--) {
            n = n * 10 + (s[i++] - '0');
        }

        if(n < 0 || 16 < n) {
            return PARSE_FAILURE;
        }

        *out_register = n;
        return i;
    }

    return PARSE_FAILURE;
}

int parse_immediate(const char *s, int *out_immediate) {
    int i = skip(s, is_space);

    if(s[i++] != '#' || s[i++] != '0' || toupper(s[i++]) != 'X') {
        return PARSE_FAILURE;
    }

    char c = s[i];
    if(isxdigit(c)) {
        int len = skip(s + i, isxdigit);
        int n = 0;
        for(; len > 0; len--) {
            c = s[i++];
            n = n * 16 + ((isdigit(c))? c - '0': toupper(c) -'A' + 10);
        }

        if(n < (int)0x80000000 || (int)0x7FFFFFFF < n) {
            return PARSE_FAILURE;
        }

        *out_immediate = n;
        return i;
    }

    return PARSE_FAILURE;
}

int is_register(const char *s) {
    int i = skip(s, is_space);
    return toupper(s[i]) == 'R';
}

int skip_comma(const char *s) {
    int i = skip(s, is_space);
    if(s[i] == ',') {
        return ++i;
    }
    return PARSE_FAILURE;
}


static int skip(const char *s, Predicate_char pred) {
    int i = 0;
    char ch = s[i];
    for(; pred(ch); ch = s[++i]);
    return i;
}

static int is_space(int c) {
    return c == ' ';
}

static int is_one_first(int c) {
    return c == '_' || isalpha(c);
}

static int is_one_rest(int c) {
    return is_one_first(c) || isdigit(c);
}


/* unit test */
static void verify_parse_one_error(char *input, ParseError expect);
static void verify_parse_one(char *input, int expect_read_len, char *expect);
static void verify_parse_register_error(char *input, ParseError expect);
static void verify_parse_register(char *input, int expect_read_len, int expect);
static void verify_parse_immediate_error(char *input, ParseError expect);
static void verify_parse_immediate(char *input, int expect_read_len, int expect);
static void verify_skip_comma(char *input, int expect);

void parser_test() {
    verify_parse_one_error("", PARSE_EOF);
    verify_parse_one_error("1", PARSE_FAILURE);
    verify_parse_one_error("  ", PARSE_EOF);

    verify_parse_one(" mov ", 4, "mov");
    verify_parse_one("mov,", 3, "mov");

    verify_parse_register_error("r", PARSE_FAILURE);
    verify_parse_register_error("b10", PARSE_FAILURE);

    verify_parse_register("r15", 3, 15);
    verify_parse_register("  r0,", 4, 0);

    verify_skip_comma("", PARSE_FAILURE);
    verify_skip_comma("mov", PARSE_FAILURE);
    verify_skip_comma(" ,r0", 2);

    verify_parse_immediate_error("0x68", PARSE_FAILURE);
    verify_parse_immediate_error("#x68", PARSE_FAILURE);
    verify_parse_immediate("#0x68", 5, 0x68);
    verify_parse_immediate("#0x1ABC", 7, 0x1ABC);
}

static void verify_parse_one_error(char *input, ParseError expect) {
    int actual = parse_one(input, NULL);
    assert(expect == actual);
}

static void verify_parse_one(char *input, int expect_read_len, char *expect) {
    Substring actual = {0};
    int actual_read_len = parse_one(input, &actual);
    assert(expect_read_len == actual_read_len);

    int eq = str_eq_subs(expect, &actual);
    assert(eq);
}

static void verify_parse_register_error(char *input, ParseError expect) {
    int actual = parse_register(input, NULL);
    assert(expect == actual);
}

static void verify_parse_register(char *input, int expect_read_len , int expect) {
    int actual = -1;
    int actual_read_len = parse_register(input, &actual);
    assert(expect_read_len == actual_read_len);
    assert(expect == actual);
}

static void verify_parse_immediate_error(char *input, ParseError expect) {
    int actual = parse_immediate(input, NULL);
    assert(expect == actual);
}

static void verify_parse_immediate(char *input, int expect_read_len, int expect) {
    int actual = -1;
    int actual_read_len = parse_immediate(input, &actual);
    assert(expect_read_len == actual_read_len);
    assert(expect == actual);
}

static void verify_skip_comma(char *input, int expect) {
    int actual = skip_comma(input);
    assert(expect == actual);
}

