#include <stdio.h>
#include "cl_getline.h"
#include "assembler.h"

void cl_getline_test();
void symbol_test();
void parser_test();
void assembler_test();


int main(int argc, char *argv[]) {

    if(argc <= 1) {
        cl_getline_test();
        symbol_test();
        parser_test();
        assembler_test();

        return 0;
    }

    if(!cl_getline_set_file(argv[1])) {
        return 1;
    }

    char bin[100*1024];
    int len = assemble(bin);
    fwrite(bin, 1, len, stdout);
}

