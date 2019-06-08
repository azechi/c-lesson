#include <string.h>
#include <stdio.h>
#include "parser.h"

int cl_getline(char **out_buf);

typedef struct Emitter_ {
    char *buff;
    int pos;
} Emitter;


char bin[100*1024];
Emitter emitter = {.buff = bin, .pos = 0};

static void emit_word(Emitter *emitter, int oneword);
static int asm_one(const char *str);


int assemble() {
    char *str;
    while(cl_getline(&str) >= 0) {
        /* TODO skip empty line */
        int word = asm_one(str);

        /* TODO skip empty word */
        emit_word(&emitter, word);
    }
    return 0;
}

static int asm_one(const char *str) {
    int read_len;
    Substring token = {0};
    read_len = parse_one(str, &token);
    /* token {nemonec, label, comment, space} */

    /* "mov r1, r2"  */
    str += read_len;
    if(str_eq_subs("mov", &token)) {
        int r1;
        read_len = parse_register(str, &r1);
        if(read_len < 0) {
            return read_len;
        }

        str += read_len;
        read_len = skip_comma(str);
        if(read_len < 0) {
            return read_len;
        }

        str += read_len;

        int r2;
        read_len = parse_register(str, &r2);
        if(read_len < 0) {
            return read_len;
        }

    }

    return 0;
}


int main() {

    parser_test();

}


#define BUF_SIZE 80
static char buf[BUF_SIZE];
int cl_getline(char **out_buf) {

    strcpy(buf, "mov r1, r2");
    *out_buf = buf;
    return strlen(buf);
}

static void emit_word(Emitter *emitter, int oneword) {
    emitter->buff[++emitter->pos] = oneword & 0xFF;
    emitter->buff[++emitter->pos] = oneword >> 8  & 0xFF;
    emitter->buff[++emitter->pos] = oneword >> 16 & 0xFF;
    emitter->buff[++emitter->pos] = oneword >> 24 & 0xFF;
}

