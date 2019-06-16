#include <assert.h>
#include <ctype.h>
#include <string.h>
#include "parser.h"

#define assert_fail(msg) assert(0&&(msg))

typedef int (*Predicate_char)(int c);

static int look_ahead(const char *s, Predicate_char pred);
static int try_skip(char const **s, Predicate_char pred);
static int try_skip_1(char const **s, Predicate_char pred);
static int try_skip_1_char(char const **s, char c);

static int is_space(int c);
static int is_one_first(int c);
static int is_one_rest(int c);
static int is_register_trailing(int c);

void skip_equal_sign(char const **s) {
    try_skip(s, is_space);
    if(!try_skip_1_char(s, '=')) {
        assert_fail("NOT EQUAL SIGN");
    }
}


void eof(char const **s) {
    try_skip(s, is_space);
    if(**s != '\0') {
        assert_fail("NOT EOF");
    }
}

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

void parse_raw_string(char const **s, char *const out_buf) {
    try_skip(s, is_space);
    try_skip_1_char(s, '"');

    enum {imm, escape, terminate} state = imm;

    int pos = 0;
    while(state != terminate) {
        char ch = *(*s)++;
        if(ch == '\0') {
            assert_fail("INVALID STRING");
        }

        switch(state) {
            case imm:
                switch(ch) {
                    case '"':
                        state = terminate;
                        break;
                    case '\\':
                        state = escape;
                        break;
                    default:
                        out_buf[pos++] = ch;
                        break;
                }
                break;
            case escape:
                switch(ch) {
                    case 'n':
                        out_buf[pos++] = '\n';
                        break;
                    case '"':
                        out_buf[pos++] = '"';
                        break;
                    case '\\':
                        out_buf[pos++] = '\\';
                        break;
                    default:
                        assert_fail("NOT INPLEMENTED");
                }
                state = imm;
                break;
            default:
                assert_fail("NOT IMPLEMENTED");
                return;
        }
    }

    out_buf[pos++] = '\0';
}

int parse_register(char const **s, int *out_register) {
    try_skip(s, is_space);
    if(!try_skip_1_char(s, 'r')) {
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

    if(!try_skip_1_char(s, '0') || !try_skip_1_char(s, 'x')) {
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

    if(!try_skip_1_char(s, '#')) {
        return 0;
    }

    int sign = try_skip_1_char(s, '-')? -1: 1;

    if(!try_skip_1_char(s, '0') || !try_skip_1_char(s, 'x')) {
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
    return try_skip_1_char(s, ',');
}

int skip_sbracket_open(char const **s) {
    try_skip(s, is_space);
    return try_skip_1_char(s, '[');
}

int skip_sbracket_close(char const **s) {
    try_skip(s, is_space);
    return try_skip_1_char(s, ']');
}

int one_is_label(const Substring *subs) {
    const char *s = subs->str + subs->len;
    return try_skip_1_char(&s, ':') && follows_eof(s);
}

int follows_eof(const char *s) {
    try_skip(&s, is_space);
    return try_skip_1_char(&s, '\0');
}

int follows_register(const char *s) {
    try_skip(&s, is_space);
    return try_skip_1_char(&s, 'r')
        && try_skip(&s, isdigit)
        && try_skip_1(&s, is_register_trailing);
}

int follows_sbracket_open(const char *s) {
    try_skip(&s, is_space);
    return try_skip_1_char(&s, '[');
}

int follows_sbracket_close(const char *s) {
    try_skip(&s, is_space);
    return try_skip_1_char(&s, ']');
}

int follows_raw_word(const char *s) {
    try_skip(&s, is_space);
    return (try_skip_1_char(&s, '0')
        && try_skip_1_char(&s, 'x'));
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

static int try_skip_1_char(char const **s, char c) {
    if(tolower(**s) != c) {
        return 0;
    }
    *s += 1;
    return 1;
}

static int is_space(int c) {
    return c == ' ' || c == '\t';
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
typedef int(ParseInt)(char const **, int*);

static void verify_parse_one(const char *input, char *expect_one, char *expect_rest);
static void verify_parse_int(ParseInt parse, const char *input, int expect, char *expect_rest);

static void verify_parse_one(const char *input, char *expect_one, char *expect_rest) {
    Substring actual = {0};
    int success = parse_one(&input, &actual);
    assert(success);

    int eq = str_eq_subs(expect_one, &actual);
    int rest_eq = (strcmp(expect_rest, input) == 0);
    assert(eq && rest_eq);
}

static void verify_parse_int(ParseInt parse, const char *input, int expect, char *expect_rest) {
    int actual = -1;
    int success = parse(&input, &actual);
    assert(success);

    int rest_eq = (strcmp(expect_rest, input) == 0);
    assert(expect == actual && rest_eq);
}

static void verify_parse_raw_string(const char *input, const char *expect) {
    char bin[20];
    parse_raw_string(&input, bin);

    int eq = strcmp(expect, bin) == 0;
    assert(eq);
}

void parser_test() {
    verify_parse_one("mov", "mov", "");
    verify_parse_one(" mov, ", "mov", ", ");

    verify_parse_int(parse_register, "r0", 0, "");
    verify_parse_int(parse_register, "r15 ", 15, " ");
    verify_parse_int(parse_register, "r2] ", 2, "] ");
    verify_parse_int(parse_register, "r3,r15", 3, ",r15");

    verify_parse_raw_string(" \" \\n \\\" \\\\ \"  ", " \n \" \\ ");
}
