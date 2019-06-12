#include <assert.h>
#include <ctype.h>
#include <string.h>
#include "parser.h"


typedef int (*Predicate_char)(int c);

static int look_ahead(const char *s, Predicate_char pred);
static int try_skip(char const **s, Predicate_char pred);
static int try_skip_1(char const **s, Predicate_char pred);
static int try_skip_char(char const **s, char c);

static int is_space(int c);
static int is_one_first(int c);
static int is_one_rest(int c);
static int is_register_trailing(int c);


int parse_one(char const **s, Substring *out_subs) {
    try_skip(s, is_space);
    if(!is_one_first(**s)) {
        return 0;
    }

    int len = look_ahead(*s, is_one_rest);
    out_subs->str = *s;
    out_subs->len = len;
    *s += len;
    return 1;
}

int parse_label(char const **s, Substring *out_subs) {
    try_skip(s, is_space);

    int len = look_ahead(*s, isalpha);
    out_subs->str = *s;
    out_subs->len = len;
    *s += len;
    return 1;
}

int parse_register(char const **s, int *out_register) {
    try_skip(s, is_space);
    if(!try_skip_char(s, 'r')) {
        return 0;
    }

    if(!isdigit(**s)) {
        return 0;
    }

    int len = look_ahead(*s, isdigit);
    int n = 0;
    for(; len > 0; len--) {
        n = n * 10 + (**s - '0');
        ++*s;
    }

    if(n < 0 || 16 < n) {
        return 0;
    }

    *out_register = n;
    return 1;
}

int parse_raw_word(char const **s, int *out_word) {
    try_skip(s, is_space);

    if(!try_skip_char(s, '0') || !try_skip_char(s, 'x')) {
        return 0;
    }

    if(isxdigit(**s)) {
        int len = look_ahead(*s, isxdigit);
        if(len != 8){
            return 0;
        }

        int n;
        char c;
        for(; len > 0; len--) {
            c = **s;
            n = (n << 4) + ((isdigit(c))? c - '0': toupper(c) - 'A' + 10);
            ++*s;
        }

        *out_word = n;
        return 1;
    }

    return 0;
}

int parse_immediate(char const **s, int *out_immediate) {
    try_skip(s, is_space);

    if(!try_skip_char(s, '#')) {
        return 0;
    }

    int sign = try_skip_char(s, '-')? -1: 1;

    if(!try_skip_char(s, '0') || !try_skip_char(s, 'x')) {
        return 0;
    }

    if(isxdigit(**s)) {
        int len = look_ahead(*s, isxdigit);
        int n = 0;
        char c;
        for(; len > 0; len--) {
            c = *(*s)++;
            n = n * 16 + ((isdigit(c))? c - '0': toupper(c) -'A' + 10);
        }
        n *= sign;

        if(n < (int)0x80000000 || (int)0x7FFFFFFF < n) {
            return 0;
        }

        *out_immediate = n;
        return 1;
    }

    return 0;
}

int skip_comma(char const **s) {
    try_skip(s, is_space);
    return try_skip_char(s, ',');
}

int skip_sbracket_open(char const **s) {
    try_skip(s, is_space);
    return try_skip_char(s, '[');
}

int skip_sbracket_close(char const **s) {
    try_skip(s, is_space);
    return try_skip_char(s, ']');
}

int one_is_label(const Substring *subs) {
    const char *s = subs->str + subs->len;
    return try_skip_char(&s, ':') && follows_eof(s);
}

int follows_eof(const char *s) {
    try_skip(&s, is_space);
    return try_skip_char(&s, '\0');
}

int follows_register(const char *s) {
    try_skip(&s, is_space);
    return try_skip_char(&s, 'r')
        && try_skip(&s, isdigit)
        && try_skip_1(&s, is_register_trailing);
}

int follows_sbracket_close(const char *s) {
    try_skip(&s, is_space);
    return try_skip_char(&s, ']');
}


static int look_ahead(const char *s, Predicate_char pred) {
    int i = 0;
    char c = s[i];
    for(; pred(c); c = s[++i]);
    return i;
}

static int try_skip(char const**s, Predicate_char pred) {
    int len = look_ahead(*s, pred);
    if(len == 0) {
        return 0;
    }
    *s += len;
    return 1;
}

static int try_skip_1(char const **s, Predicate_char pred) {
    if(!pred(**s)) {
        return 0;
    }

    *s += 1;
    return 1;
}

static int try_skip_char(char const **s, char c) {
    if(tolower(**s) != c) {
        return 0;
    }
    *s += 1;
    return 1;
}

static int is_space(int c) {
    return c == ' ';
}

static int is_one_first(int c) {
    return c == '_' || c == '.' || isalpha(c);
}

static int is_one_rest(int c) {
    return is_one_first(c) || isdigit(c);
}

static int is_register_trailing(int c) {
    return is_space(c) || c == ',' || c == ']' || c == '\0';
}


/* unit test */
static void verify_parse_one_failure(const char *input);
static void verify_parse_one(const char *input, char *expect_one, char *expect_rest);

typedef int(ParseInt)(char const **, int*);
static void verify_parse_int_failure(ParseInt parse, const char *input);
static void verify_parse_int(ParseInt parse, const char *input, int expect, char *expect_rest);

static void verify_parse_one_failure(const char *input) {
    int success = parse_one(&input, NULL);
    assert(!success);
}

static void verify_parse_one(const char *input, char *expect_one, char *expect_rest) {
    Substring actual = {0};
    int success = parse_one(&input, &actual);
    assert(success);

    int eq = str_eq_subs(expect_one, &actual);
    int rest_eq = (strcmp(expect_rest, input) == 0);
    assert(eq && rest_eq);
}

static void verify_parse_int_failure(ParseInt parse, const char *input) {
    int success = parse(&input, NULL);
    assert(!success);
}

static void verify_parse_int(ParseInt parse, const char *input, int expect, char *expect_rest) {
    int actual = -1;
    int success = parse(&input, &actual);
    assert(success);

    int rest_eq = (strcmp(expect_rest, input) == 0);
    assert(expect == actual && rest_eq);
}

void parser_test() {
    /* parse_one */
    verify_parse_one_failure("");
    verify_parse_one_failure(" 1");

    verify_parse_one("mov", "mov", "");
    verify_parse_one(" mov, ", "mov", ", ");

    /* parse_register */
    verify_parse_int_failure(parse_register, "");
    verify_parse_int_failure(parse_register, "r ");
    verify_parse_int_failure(parse_register, "b10");

    verify_parse_int(parse_register, "r0", 0, "");
    verify_parse_int(parse_register, "r15 ", 15, " ");
    verify_parse_int(parse_register, "r2] ", 2, "] ");
    verify_parse_int(parse_register, "r3,r15", 3, ",r15");

    /* parse_raw_word */



}
