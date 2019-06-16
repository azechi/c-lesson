#ifndef PARSER_H
#define PARSER_H

#include "substring.h"

/* return value is boolean */
int parse_one(char const **s, Substring *out_subs);
void parse_raw_string(char const **s, char *const out_buf);
int parse_register(char const **s, int *out_register);
int parse_raw_word(char const **s, int *out_word);
int parse_immediate(char const **s, int *out_immediate);

void parse_register_list(char const **s, int *out_bits);

/* return value is boolean */
int skip_comma(char const **s);
int skip_sbracket_open(char const **s);
int skip_sbracket_close(char const **s);

int try_skip_bang(char const **s);

void skip_equal_sign(char const **s);

void eof(char const **s);

/* return value is boolean */
int one_is_label(const Substring *subs);

/* return value is boolean */
int follows_eof(const char *s);
int follows_register(const char *s);
int follows_sbracket_open(const char *s);
int follows_sbracket_close(const char *s);
int follows_raw_word(const char *s);


#endif
