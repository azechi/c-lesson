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
int parse_immediate(const char *s, int *out_immediate);
int parse_raw_word(const char *s, int *out_word);
int skip_comma(const char *s);
int skip_sbracket_open(const char *s);
int skip_sbracket_close(const char *s);

int is_register(const char *s);
int is_sbracket_close(const char *s);


void parser_test();

#endif
