#include <assert.h>
#include <stdlib.h>

#include "cl_getline.h"
#include "parser.h"
#include "assembler.h"

#define assert_fail(msg) assert(0&&(msg))

static int asm_one(const char *s, int *out_word);
static void emit_word(Emitter *emitter, int oneword);


int assemble(Emitter *emitter) {
    char *str;
    while(cl_getline(&str) >= 0) {
        int word;
        if(!asm_one(str, &word)) {
            return 0;
        }

        if(word) {
            /* skip empty line, empty word */
            emit_word(emitter, word);
        }
    }
    return 1;
}

static int asm_one(const char *s, int *out_word) {

    int i;
    Substring subs = {0};
    i = parse_one(s, &subs);
    if(i == PARSE_FAILURE) {
        return 0;
    }
    if(i == PARSE_EOF) {
        *out_word = 0;
        return 1;
    }

    s += i;

    if(str_eq_subs(".raw", &subs)) {
        int word;
        if((i = parse_raw_word(s, &word)) < 0) {
            return 0;
        }
        *out_word = word;
        return 1;
    } else if(str_eq_subs("ldr", &subs) || str_eq_subs("str", &subs)) {

        int l_bit = str_eq_subs("ldr", &subs)? 0x00100000: 0;

        int rd;
        if((i = parse_register(s, &rd)) < 0) {
            return 0;
        }
        s += i;

        if((i = skip_comma(s)) < 0) {
            return 0;
        }
        s += i;

        if((i = skip_sbracket_open(s)) < 0) {
            return 0;
        }
        s += i;

        int rn;
        if((i = parse_register(s, &rn)) < 0) {
            return 0;
        }
        s += i;

        if(is_sbracket_close(s)) {
            if((i = skip_sbracket_close(s)) < 0) {
                return 0;
            }
            s += i;
            *out_word = 0xE5800000 + l_bit + (rn << 16) + (rd << 12);
            return 1;
        }

        if((i = skip_comma(s)) < 0) {
            return 0;
        }
        s += i;

        int offset;
        if((i = parse_immediate(s, &offset)) < 0) {
            return 0;
        }
        s += i;

        int u_bit = (offset < 0)? 0: 0x00800000;

        *out_word = 0xE5000000 + l_bit + u_bit + (rn << 16) + (rd << 12) + (abs(offset) & 0xFFF);
        return 1;

    } else if(str_eq_subs("mov", &subs)) {
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

static void emit_word(Emitter *emitter, int word) {
    emitter->buf[emitter->pos++] = word & 0xFF;
    emitter->buf[emitter->pos++] = word >> 8 & 0xFF;
    emitter->buf[emitter->pos++] = word >> 16 & 0xFF;
    emitter->buf[emitter->pos++] = word >> 24 & 0xFF;
}

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

    verify_asm_one(".raw 0x12345678", 0x12345678);
    verify_asm_one(".raw 0x80000001", 0x80000001);

    verify_asm_one("ldr r1, [r15, #0x30]", 0xE59F1030);
    verify_asm_one("ldr r1, [r15, #-0x30]", 0xE51F1030);
    verify_asm_one("ldr r1, [r15]", 0xE59F1000);
    verify_asm_one("str r0, [r1]", 0xE5810000);
}


