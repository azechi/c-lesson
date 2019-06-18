#ifndef PARSER_H
#define PARSER_H

#include "substring.h"


int follows_eol(const char *s);

int try_parse_one(char const **str, Substring *out_subs);
int one_is_label(const Substring *one);

int try_parse_string_eol(char const **str, char *const out_buf);
int try_parse_imm_eol(char const **str, int *out_val);

int try_parse_label_eol(char const **str, Substring *out_label);
int try_parse_rn_register_list_eol(char const **str, int *out_rn, int *out_write_back ,int *out_bits);

int try_parse_rd_rm_eol(char const **str, int *out_rd, int *out_rm);
int try_parse_rd_rm_rs_eol(char const **str, int *out_rd, int *out_rm, int *out_rs);
int try_parse_rx_imm_eol(char const **str, int *out_rx, int *out_imm);
int try_parse_rd_rn_imm_eol(char const **str, int *out_rd, int *out_rn, int *out_imm);

int try_parse_rd_expr_eol(char const **str, int *out_rd, int *out_val, Substring *out_expr);
int try_parse_rd_label_eol(char const **str, int *out_rd, Substring *out_label, Substring *out_expr);
int try_parse_rd__rn__eol(char const **str, int *out_rd, int *out_rn);
int try_parse_rd__rn_imm__eol(char const **str, int *out_rd, int *out_rn, int *out_imm);


#endif
