#include <string.h>
#include <stdio.h>
#include "assembler.h"
#include "parser.h"


/*
#define BUF_SIZE 80
static char buf[BUF_SIZE];
static int cl_getline(char **out_buf);
*/

int main() {

    parser_test();
    assembler_test();

}

/*
int cl_getline(char **out_buf) {

    strcpy(buf, "mov r1, r2");
    *out_buf = buf;
    return strlen(buf);
}
*/
