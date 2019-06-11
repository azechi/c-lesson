#include <assert.h>
#include <stdlib.h>

#include "cl_getline.h"
#include "parser.h"
#include "symbol.h"
#include "label_dictionary.h"
#include "assembler.h"


#define assert_fail(msg) assert(0&&(msg))

typedef enum AssembleState_ {
    ASM_ERROR,
    ASM_INST,
    ASM_LABEL,
    ASM_BLANK
} AssembleState;

static AssembleState asm_one(const char *s, int *out_word);
static void emit_word(Emitter *emitter, int oneword);


int assemble(Emitter *emitter) {
    char *str;
    while(cl_getline(&str) >= 0) {
        int word;
        switch(asm_one(str, &word)) {
            case ASM_INST:
                emit_word(emitter, word);
                break;
            case ASM_LABEL:
                label_dict_put(word, emitter->pos);
                break;
            case ASM_BLANK:
                break;
            case ASM_ERROR:
            default:
                return 0;
        }
    }
    return 1;
}

static AssembleState asm_one(const char *s, int *out_word) {

    if(follows_eof(s)) {
        /* blank line */
        return ASM_BLANK;
    }

    Substring one = {0};
    if(!parse_one(&s, &one)) {
        return ASM_ERROR;
    }

    if(one_is_label(&one)) {
        if(find_label_symbol(&one)) {
            /* duplicate label */
            return ASM_ERROR;
        }

        *out_word = to_label_symbol(&one);
        return ASM_LABEL;
    }

    int mnemonic = to_mnemonic_symbol(&one);
    if(mnemonic == mnemonic_raw) {
        int word;
        if(parse_raw_word(&s, &word) && follows_eof(s)) {
            *out_word = word;
            return ASM_INST;
        }
    } else if(mnemonic_ldr == mnemonic || mnemonic_str == mnemonic) {
        int l_bit = str_eq_subs("ldr", &one)? 0x00100000: 0;

        int rd, rn;
        if(parse_register(&s, &rd)
            && skip_comma(&s)
            && skip_sbracket_open(&s)
            && parse_register(&s, &rn)) {

            if(follows_sbracket_close(s)) {
                if(skip_sbracket_close(&s)
                    && follows_eof(s)) {

                    *out_word = 0xE5800000 + l_bit + (rn << 16) + (rd << 12);
                    return ASM_INST;
                }
            } else {
                int offset;
                if(skip_comma(&s)
                    && parse_immediate(&s, &offset)
                    && skip_sbracket_close(&s)
                    && follows_eof(s)) {

                    int u_bit = (offset < 0)? 0: 0x00800000;
                    *out_word = 0xE5000000 + l_bit + u_bit + (rn << 16) + (rd << 12) + (abs(offset) & 0xFFF);
                    return ASM_INST;
                }
            }
        }
    } else if(mnemonic_mov == mnemonic) {
        int rd;
        if(parse_register(&s, &rd)
            && skip_comma(&s)) {

            if(follows_register(s)) {
                int rm;
                if(parse_register(&s, &rm)
                    && follows_eof(s)) {
                    *out_word = 0xE1A00000 + (rd << 12) + rm;
                    return ASM_INST;
                }
            } else {
                int imm;
                if(parse_immediate(&s, &imm)
                    && follows_eof(s)) {
                    /* not implement 4bit rotate. only 8 bit immediate value is supported*/
                    if(imm < 0 || 0xFF < imm) {
                        return ASM_ERROR;
                    }

                    *out_word = 0xE3A00000 + (rd << 12) + imm;
                    return ASM_INST;
                }
            }
        }
    }


    return ASM_ERROR;
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

    verify_asm_one("loop: ", 0x00010001);
}

