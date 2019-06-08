#include <assert.h>

#include "parser.h"
#include "assembler.h"

#define assert_fail(msg) assert(0&&(msg))

typedef struct Emitter_ {
    char *buff;
    int pos;
} Emitter;


char bin[100*1024];
Emitter emitter = {.buff = bin, .pos = 0};

static int asm_one(const char *s, int *out_word) {

    int i;
    Substring subs = {0};
    i = parse_one(s, &subs);
    if(i == PARSE_EOF || i == PARSE_FAILURE) {
        return 0;
    }
    s += i;

    if(str_eq_subs("mov", &subs)) {
        int rd;
        if((i = parse_register(s, &rd)) < 0) {
            return 0;
        }
        s += i;

        if((i = skip_comma(s)) < 0) {
            return 0;
        }
        s += i;

        if(is_register(s)) {
            int rm;
            if((i = parse_register(s, &rm)) < 0) {
                return 0;
            }

            *out_word = 0xE1A00000 + (rd << 12) + rm;
            return 1;
        } else {
            int imm;
            if((i = parse_immediate(s, &imm)) < 0) {
                return 0;
            }

            /* not implement 4bit rotate. only 8 bit immediate value is supported*/
            if(imm < 0 || 0xFF < imm) {
                return 0;
            }

            *out_word = 0xE3A00000 + (rd << 12) + imm;
            return 1;
        }

    }

    return 0;
}

/*

static void emit_word(Emitter *emitter, int oneword);

int assemble() {
    char *str;
    while(cl_getline(&str) >= 0) {
         TODO skip empty line
        int word = asm_one(str);

         TODO skip empty word
        emit_word(&emitter, word);
    }
    return 0;
}
*/

/*
static void emit_word(Emitter *emitter, int oneword) {
    emitter->buff[++emitter->pos] = oneword & 0xFF;
    emitter->buff[++emitter->pos] = oneword >> 8  & 0xFF;
    emitter->buff[++emitter->pos] = oneword >> 16 & 0xFF;
    emitter->buff[++emitter->pos] = oneword >> 24 & 0xFF;
}
*/


/* unit test */
static void verify_asm_one(const char *input, int expect) {
    int actual;
    int success = asm_one(input, &actual);
    assert(success);
    assert(expect == actual);
}

void assembler_test() {
    verify_asm_one("mov r1, r2", 0xE1A01002);
    verify_asm_one(" mov r15 ,  r0  ", 0xE1A0F000);
    verify_asm_one("mov r1, #0x68", 0xE3A01068);
}

