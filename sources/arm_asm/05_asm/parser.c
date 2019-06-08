#include <assert.h>
#include <string.h>
#include "parser.h"

typedef int (*Predicate_char)(char ch);
static int skip(const char *s, Predicate_char pred);
static int is_space(char c);
static int is_digit(char c);
static int is_one_first(char c);
static int is_one_rest(char c);


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

    if(c != 'r') {
        return PARSE_FAILURE;
    }
    c = s[++i];

    if(is_digit(c)) {
        int len = skip(s + i, is_digit);
        int n = 0;
        for(; len > 0; len--) {
            n = n * 10 + (s[i++] - '0');
        }
        *out_register = n;
        return i;
    }

    return PARSE_FAILURE;
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

static int is_space(char c) {
    return c == ' ';
}

static int is_digit(char c) {
    return '0' <= c && c <= '9';
}

static int is_one_first(char c) {
    return c == '_' || ('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z');
}

static int is_one_rest(char c) {
    return is_one_first(c) || is_digit(c);
}


/* unit test */
static void verify_parse_one_error(char *input, ParseError expect);
static void verify_parse_one(char *input, int expect_read_len, char *expect);
static void verify_parse_register_error(char *input, ParseError expect);
static void verify_parse_register(char *input, int expect_read_len , int expect);
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

static void verify_skip_comma(char *input, int expect) {
    int actual = skip_comma(input);
    assert(expect == actual);
}

