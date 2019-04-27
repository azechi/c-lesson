#include "clesson.h"
#include <assert.h>

/*
cc cl_getc.c int_parser_getc.c
*/

enum TokenType { NUMBER, SPACE };

int parse_one(int c, int *out_val, int *out_type);

int main() {
    int answer1 = 0;
    int answer2 = 0;

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

    /*
    // sample for cl_getc() usage.
    int c;

    while((c = cl_getc()) != EOF) {
        printf("%c\n",c );
    }
    */

    // verity result.
    assert(answer1 == 123);
    assert(answer2 == 456);

    return 1;
}

int parse_one(int c, int *out_val, int *out_type){
    
    int d;

    if (c == EOF){
        c = cl_getc();
    }
    d = cl_getc();

    if (('0' <= c && c <= '9')){

        int acm = c - '0';

        while('0' <= d && d <= '9'){
            acm = acm * 10 + d - '0';
            d = cl_getc();
        }

        *out_type = NUMBER;
        *out_val = acm;
    } else if (c == ' ') {

        while(d == ' '){
            d = cl_getc();
        }
    
        *out_type = SPACE;
        *out_val = ' ';
    } else {
        *out_type = -1;
        *out_val = EOF;
    }
        
    return d;

}
