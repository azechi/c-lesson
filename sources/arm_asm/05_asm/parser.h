#ifndef PARSER_H
#define PARSER_H

#include "substring.h"


int try_parse_rd_rm_eol(char const **str, int *out_rd, int *out_rm);
int try_parse_rd_rm_rs_eol(char const **str, int *out_rd, int *out_rm, int *out_rs);
int try_parse_rx_imm_eol(char const **str, int *out_rx, int *out_imm);
int try_parse_rd_rn_imm_eol(char const **str, int *out_rd, int *out_rn, int *out_imm);

int try_parse_rd_expr_eol(char const **str, int *out_rd, int *out_val, Substring *out_expr);
int try_parse_rd_label_eol(char const **str, int *out_rd, Substring *out_label, Substring *out_expr);
int try_parse_rd__rn__eol(char const **str, int *out_rd, int *out_rn);
int try_parse_rd__rn_imm__eol(char const **str, int *out_rd, int *out_rn, int *out_imm);







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
