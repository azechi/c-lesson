#ifndef PARSER_H
#define PARSER_H

#include <stdio.h>

typedef enum LexicalType_ {
    LEX_UNKNOWN = 0,
    LEX_NUMBER,
    LEX_SPACE,
    LEX_EXECUTABLE_NAME,
    LEX_LITERAL_NAME,
    LEX_OPEN_CURLY,
    LEX_CLOSE_CURLY,
    LEX_END_OF_FILE
} LexicalType;


typedef struct Token_ {
    enum LexicalType_ ltype;
    union {
        int number;
        char onechar;
        char *name;
    } u;
} Token;

/*
   return one character and move cursor.
   return EOF if end of file.
   */
int cl_getc();
void cl_getc_set_src(const char *str);
void cl_getc_set_file(FILE *input_fp);


int parse_one(int prev_ch, Token *out_token);

void parser_test_all();


#endif
