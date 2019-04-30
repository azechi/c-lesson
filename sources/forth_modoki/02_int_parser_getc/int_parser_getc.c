#include "clesson.h"
#include <assert.h>

/*
cc cl_getc.c int_parser_getc.c
*/

enum TokenType { NUMBER, SPACE };

int parse_one(int c, int *out_val, int *out_type);

void test_parse_one_123();
void test_parse_one_123_456();

int main() {
    int answer1 = 0;
    int answer2 = 0;

    // UnitTest
    test_parse_one_123();
    test_parse_one_123_456();

    cl_getc_set_src("123 456");
    // write something here.
    int c = EOF;
    int out_val;
    int out_type;

    int* ans[] = {&answer1, &answer2};
    int p = 0;
    
    do {

        c = parse_one(c, &out_val, &out_type);
        if (out_type == NUMBER){
            *ans[p++] = out_val; 
        }

    } while (c != EOF);


    // verity result.
    assert(answer1 == 123);
    assert(answer2 == 456);

    return 1;
}

int parse_one(int prev_ch, int *out_val, int *out_type){
    
    int ch;

    if (prev_ch == EOF){
        prev_ch = cl_getc();
    }
    ch = cl_getc();

    if (('0' <= prev_ch && prev_ch <= '9')){

        int acm = prev_ch - '0';

        while('0' <= ch && ch <= '9'){
            acm = acm * 10 + ch - '0';
            ch = cl_getc();
        }

        *out_type = NUMBER;
        *out_val = acm;
    } else if (prev_ch == ' ') {

        while(ch == ' '){
            ch = cl_getc();
        }
    
        *out_type = SPACE;
        *out_val = ' ';
    } else {
        *out_type = -1;
        *out_val = EOF;
    }
        
    return ch;

}

void test_parse_one_123() {

    cl_getc_set_src("123");

    int c, v, t;

    c = parse_one(EOF, &v, &t);

    assert(c == EOF);
    assert(v == 123);
    assert(t == NUMBER);

}

void test_parse_one_123_456() {

    cl_getc_set_src("123 456");

    int c, v, t;

    c = parse_one(EOF, &v, &t);
    
    assert(c == ' ');
    assert(v == 123);
    assert(t == NUMBER);

    c = parse_one(c, &v, &t);

    assert(c == '4');
    assert(v == ' ');
    assert(t == SPACE);
    
    c = parse_one(c, &v, &t);

    assert(c == EOF);
    assert(v == 456);
    assert(t == NUMBER);

}

