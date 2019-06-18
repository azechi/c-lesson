#include <assert.h>
#include <ctype.h>
#include <string.h>
#include "parser.h"

#define assert_fail(msg) assert(0&&(msg))

typedef int (*Predicate_char)(int c);


static void skip_blank(char const **str);

static int try_skip_char(char const **str, char ch);
static int try_parse_register(char const **str, int *out_rn);
static int try_parse_register_list(char const **str, int *out_bits);
static int try_parse_string(char const **str, char *const out_buf);
static int try_parse_immediate_value(char const **str, int *out_val);

static int try_ex_parse_label(char const **str, Substring *out_label);
static int try_ex_parse_int_10base(char const **str, int *out_val);
static int try_ex_parse_int_16base(char const **str, int *out_val);

static int try_ex_skip_string(char const **str, const char *expect);
static int try_ex_skip_char(char const **str, char ch);
static int try_ex_skip_1(char const **str, Predicate_char pred);
static int try_ex_skip(char const **str, Predicate_char pred);

static int look_ahead(const char *s, Predicate_char pred);

static int is_blank(int c);
static int is_one_first(int c);
static int is_one_rest(int c);


int follows_eol(const char *s) {
    skip_blank(&s);
    return *s == '\0';
}

int try_parse_one(char const **str, Substring *out_subs) {
    const char *s = *str;
    skip_blank(&s);

    if(!is_one_rest(*s)) {
        return 0;
    }
    
    int len = look_ahead(s, is_one_rest);
    out_subs->str = s;
    out_subs->len = len;

    *str = s + len;
    return 1;
}

int one_is_label(const Substring *subs) {
    const char *s = subs->str + subs->len;
    return try_ex_skip_char(&s, ':') && follows_eol(s);
}

int try_parse_string_eol(char const **str, char *const out_buf) {
    const char *s = *str;
    if(try_parse_string(&s, out_buf) 
        && follows_eol(s)) {
        *str = s;
        return 1;
    }

    return 0;
}

int try_parse_imm_eol(char const **str, int *out_val) {
    const char *s = *str;
    skip_blank(&s);
    if(try_ex_parse_int_16base(&s, out_val)
        && follows_eol(s)) {
        *str = s;
        return 1;
    }
    return 0;
}

int try_parse_label_eol(char const **str, Substring *out_label) {
    const char *s = *str;
    skip_blank(&s);
    if(try_ex_parse_label(&s, out_label)
        && follows_eol(s)) {
        *str = s;
        return 1;
    }
    return 0;
}

int try_parse_rn_register_list_eol(char const **str, int *out_rn, int *out_write_back ,int *out_bits) {
    const char *s = *str;
    if(!try_parse_register(&s, out_rn)) {
        return 0;
    }

    *out_write_back = try_ex_skip_char(&s, '!');

    if(try_skip_char(&s, ',')
        && try_parse_register_list(&s, out_bits)
        && follows_eol(s)) {
        *str = s;
        return 1;
    }
    return 0;
}

























int try_parse_rd_rm_eol(char const **str, int *out_rd, int *out_rm) {
    const char *s = *str;
    
    if(try_parse_register(&s, out_rd)
        && try_skip_char(&s, ',')
        && try_parse_register(&s, out_rm)
        && follows_eol(s)) {

        *str = s;
        return 1;
    }

    return 0;
}

int try_parse_rd_rm_rs_eol(char const **str, int *out_rd, int *out_rm, int *out_rs) {
    const char *s = *str;
    
    if(try_parse_register(&s, out_rd)
        && try_skip_char(&s, ',')
        && try_parse_register(&s, out_rm)
        && try_skip_char(&s, ',')
        && try_parse_register(&s, out_rs)
        && follows_eol(s)) {

        *str = s;
        return 1;
    }

    return 0;
}

int try_parse_rx_imm_eol(char const **str, int *out_rx, int *out_imm) {
    const char *s = *str;
    
    if(try_parse_register(&s, out_rx)
        && try_skip_char(&s, ',')
        && try_parse_immediate_value(&s, out_imm)
        && follows_eol(s)) {

        *str = s;
        return 1;
    }

    return 0;
}

int try_parse_rd_rn_imm_eol(char const **str, int *out_rd, int *out_rn, int *out_imm) {
    const char *s = *str;
    
    if(try_parse_register(&s, out_rd)
        && try_skip_char(&s, ',')
        && try_parse_register(&s, out_rn)
        && try_skip_char(&s, ',')
        && try_parse_immediate_value(&s, out_imm)
        && follows_eol(s)) {

        *str = s;
        return 1;
    }

    return 0;
}

int try_parse_rd_expr_eol(char const **str, int *out_rd, int *out_val, Substring *out_expr) {
    /* r1, =0x12345678 */
    const char *s = *str;
    if(try_parse_register(&s, out_rd)
        && try_skip_char(&s, ',')
        && try_skip_char(&s, '=')) {

        const char *begin = s - 1;
        out_expr->str = begin;

        if(try_ex_parse_int_16base(&s, out_val)
            && follows_eol(s)){

            out_expr->len = s - begin;
            *str = s;
            return 1;
        }
    }

    return 0;
}

int try_parse_rd_label_eol(char const **str, int *out_rd, Substring *out_label, Substring *out_expr) {
    /* r1, =label */
    const char *s = *str;
    if(try_parse_register(&s, out_rd)
        && try_skip_char(&s, ',')
        && try_skip_char(&s, '=')) {

        const char *begin = s - 1;
        out_expr->str = begin;

        if(try_ex_parse_label(&s, out_label)
            && follows_eol(s)){

            out_expr->len = s - begin;
            *str = s;
            return 1;
        }
    }

    return 0;
}

int try_parse_rd__rn__eol(char const **str, int *out_rd, int *out_rn) {
    /* rd, [rn] */
    const char *s = *str;
    if(try_parse_register(&s, out_rd)
        && try_skip_char(&s, ',')
        && try_skip_char(&s, '[')
        && try_parse_register(&s, out_rn)
        && try_skip_char(&s, ']')
        && follows_eol(s)) {
        
        *str = s;
        return 1;
    }

    return 0;
}

int try_parse_rd__rn_imm__eol(char const **str, int *out_rd, int *out_rn, int *out_imm) {
    /* rd, [rn, #imm] */
    const char *s = *str;
    if(try_parse_register(&s, out_rd)
        && try_skip_char(&s, ',')
        && try_skip_char(&s, '[')
        && try_parse_register(&s, out_rn)
        && try_skip_char(&s, ',')
        && try_parse_immediate_value(&s, out_imm)
        && try_skip_char(&s, ']')
        && follows_eol(s)) {
        
        *str = s;
        return 1;
    }

    return 0;

}


static void skip_blank(char const **str) {
    *str += look_ahead(*str, is_blank);
}

static int try_skip_char(char const **str, char ch) {
    const char *s = *str;
    skip_blank(&s);
    if(try_ex_skip_char(&s, ch)){
        *str = s;
        return 1;
    }
    return 0;
}

static int try_parse_register(char const **str, int *out_rn) {
    const char *s = *str;
    skip_blank(&s);

    int i;
    if(try_skip_char(&s, 'r') 
        && try_ex_parse_int_10base(&s, &i)
        && (0 <= i && i <= 16)) {
        
        *out_rn = i;
        *str = s;
        return 1;
    }
    return 0;
}

static int try_parse_register_list(char const **str, int *out_bits) {
    /* {r0, ... ,r15  } */
    const char *s = *str;
    skip_blank(&s);
    if(try_ex_skip_char(&s, '{')) {
        int acm = 0;
        do {
            int i;
            if(!try_parse_register(&s, &i)) {
                return 0;
            }
            acm += 1 << i;
        } while(try_skip_char(&s, ','));

        try_skip_char(&s, '}');
        *out_bits = acm;
        *str = s;
        return 1;
    }

    return 0;
}

static int try_parse_string(char const **str, char *const out_buf) {
    /* "string" */
    const char *s = *str;
    
    skip_blank(&s);
    if(!try_ex_skip_char(&s, '"')) {
        return 0;
    }

    enum {IMM, ESC, FIN} state = IMM;
    int pos = 0;
    while(state != FIN) {
        char ch = *(s++);
        if(ch == '\0') {
            return 0;
        }

        switch(state) {
            case IMM:
                switch(ch) {
                    case '"':
                        state = FIN;
                        break;
                    case '\\':
                        state = ESC;
                        break;
                    default:
                        out_buf[pos++] = ch;
                        break;
                }
                break;
            case ESC:
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
                        return 0;
                }
                state = IMM;
                break;
            default:
                assert_fail("NOT IMPLEMENTED");
                return 0;
        }
    }

    out_buf[pos++] = '\0';
    *str = s;
    return 1;
}

static int try_parse_immediate_value(char const **str, int *out_val) {
    /* #1, #-1, #0xFF, #-0xFF */
    
    const char *s = *str;
    skip_blank(&s);
    if(!try_ex_skip_char(&s, '#')) {
        return 0;
    }

    int sign = try_ex_skip_char(&s, '-')? -1: 1;
    
    int i;
    if(try_ex_parse_int_16base(&s, &i) || try_ex_parse_int_10base(&s, &i)) {
       *out_val = i * sign;
       *str = s;
       return 1;
    }

    return 0;
}

static int try_ex_parse_label(char const **str, Substring *out_label) {
    const char *s = *str;
    if(try_ex_skip_1(&s, is_one_first)
        && try_ex_skip(&s, is_one_rest)) {
        out_label->str = *str;
        out_label->len = s - *str;
        *str = s;
        return 1;
    }

    return 0;
}

static int try_ex_parse_int_10base(char const **str, int *out_val) {
    const char *s = *str;
    if(try_ex_skip_string(&s, "0x")) {
        return 0;
    }

    if(!isdigit(*s)){
        return 0;
    }

    int len = look_ahead(s, isdigit);
    int acm = 0;
    for(; len > 0; len--) {
        acm = acm * 10 + (*s - '0');
        ++s;
    }
    
    *out_val = acm;
    *str = s;
    return 1;
}

static int try_ex_parse_int_16base(char const **str, int *out_val) {
    const char *s = *str;
    if(!try_ex_skip_string(&s, "0x")) {
        return 0;
    }

    int len = look_ahead(s, isxdigit);
    if(len > 8){
        return 0;
    }

    int acm = 0;
    char c;
    for(; len > 0; len--) {
        c = *s;
        acm = (acm << 4) + ((isdigit(c))? c - '0': toupper(c) - 'A' + 10);
        ++s;
    }

    *out_val = acm;
    *str = s;
    return 1;
}

static int try_ex_skip_string(char const **str, const char *expect) {
    const char *s = *str;
    int len = strlen(expect);
    for(; len > 0; len--) {
        if(tolower(*s++) != tolower(*expect++)) {
            return 0;
        }
    }
    *str = s;
    return 1;
}

static int try_ex_skip_char(char const **str, char ch) {
    if(tolower(**str) != ch) {
        return 0;
    }

    ++*str;
    return 1;
}

static int try_ex_skip_1(char const **str, Predicate_char pred) {
    if(!pred(**str)) {
        return 0;
    }

    ++*str;
    return 1;
}

static int try_ex_skip(char const **str, Predicate_char pred) {
    int len = look_ahead(*str, pred);
    if(len == 0) {
        return 0;
    }

    *str += len;
    return 1;
}

static int look_ahead(const char *s, Predicate_char pred) {
    int i = 0;
    char c = s[i];
    for(; pred(c); c = s[++i]);
    return i;
}

static int is_blank(int c) {
    return c == ' ' || c == '\t';
}

static int is_one_first(int c) {
    return c == '_' || c == '.' || isalpha(c);
}

static int is_one_rest(int c) {
    return is_one_first(c) || isdigit(c);
}

/* unit test */
typedef int(ParseInt)(char const **, int*);

static void verify_parse_one(const char *input, char *expect_one, char *expect_rest);
static void verify_parse_int(ParseInt parse, const char *input, int expect, char *expect_rest);

static void verify_parse_one(const char *input, char *expect_one, char *expect_rest) {
    Substring actual = {0};
    int success = try_parse_one(&input, &actual);
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
    try_parse_string_eol(&input, bin);

    int eq = strcmp(expect, bin) == 0;
    assert(eq);
}

static void verify_parse_rx_imm_eol(const char *input, int expect_rx, int expect_imm) {
    int actual_rx, actual_imm;
    int success = try_parse_rx_imm_eol(&input, &actual_rx, &actual_imm);
    assert(success && expect_rx == actual_rx && expect_imm == actual_imm);
}

void parser_test() {

    verify_parse_rx_imm_eol("r1, #0x10", 1, 0x10);

    verify_parse_one("mov", "mov", "");
    verify_parse_one(" mov, ", "mov", ", ");

    verify_parse_int(try_parse_register, "r0", 0, "");
    verify_parse_int(try_parse_register, "r15 ", 15, " ");
    verify_parse_int(try_parse_register, "r2] ", 2, "] ");
    verify_parse_int(try_parse_register, "r3,r15", 3, ",r15");

    verify_parse_raw_string(" \" \\n \\\" \\\\ \"  ", " \n \" \\ ");
}
