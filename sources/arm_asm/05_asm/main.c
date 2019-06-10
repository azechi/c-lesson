#include <stdio.h>
#include "cl_getline.h"
#include "symbol.h"
#include "assembler.h"
#include "parser.h"


int main(int argc, char *argv[]) {

    if(argc <= 1) {
        symbol_test();
        parser_test();
        assembler_test();

        return 0;
    }

    if(!cl_getline_set_file(argv[1])) {
        return 1;
    }

    char bin[100*1024];
    Emitter emitter = {.buf = bin, .pos = 0};
    init_symbol();
    if(!assemble(&emitter)) {
        return 1;
    }

    fwrite(emitter.buf, sizeof(char), emitter.pos, stdout);
}

