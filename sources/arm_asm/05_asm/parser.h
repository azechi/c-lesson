#ifndef PARSER_H
#define PARSER_H


typedef enum ParseError_ {
    /* SUCCESS >= 0 */
    PARSE_EOF = -1,
    PARSE_FAILURE = -2
} ParseError;

typedef struct Substring_ {
    const char *str;
    int len;
} Substring;

int str_eq_subs(const char *s, const Substring *subs);

int parse_one(const char *s, Substring *out_subs);
int parse_register(const char *s, int *out_register);
int skip_comma(const char *s);

void parser_test();

#endif
