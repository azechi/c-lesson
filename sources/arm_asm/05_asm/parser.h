#ifndef PARSER_H
#define PARSER_H

#include "substring.h"

/* return value is boolean */
int parse_one(char const **s, Substring *out_subs);
int parse_register(char const **s, int *out_register);
int parse_raw_word(char const **s, int *out_word);
int parse_immediate(char const **s, int *out_immediate);

/* return value is boolean */
int skip_comma(char const **s);
int skip_sbracket_open(char const **s);
int skip_sbracket_close(char const **s);

/* return value is boolean */
int one_is_label(const Substring *subs);

/* return value is boolean */
int follows_eof(const char *s);
int follows_register(const char *s);
int follows_sbracket_close(const char *s);

void parser_test();

#endif
