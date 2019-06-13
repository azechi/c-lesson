#include <assert.h>
#include <stdlib.h>
#include "cl_getline.h"
#include "parser.h"
#include "assembler_symbol_dict.h"
#include "assembler.h"
#include "assembler_emitter.h"
#include "assembler_label_dict.h"
#include "assembler_private.h"
#include "assembler_unresolve_address_list.h"

#include <string.h>

#define assert_fail(msg) assert(0&&(msg))


int MOV;
int RAW;
int LDR;
int STR;
int B;


static void prepare_mnemonic_symbol();
static void asm_one(Emitter *emitter, const char *s);
static void asm_address(UnresolveAddress *lr, int *out_word);

int assemble(char *const bin) {
    Emitter *emitter = emitter_new(bin);
    prepare_mnemonic_symbol();
    char *str;
    while(cl_getline(&str) >= 0) {
        asm_one(emitter, str);
    }

    UnresolveAddress lr;
    while(unresolve_address_pop(&lr)) {
        int word = 0;
        asm_address(&lr, &word);
        emitter_patch_word(emitter, lr.address, word);
    }

    int len = emitter_current_address(emitter);
    emitter_free(emitter);
    return len;
}


static void asm_address(UnresolveAddress *lr, int *out_word) {
    int dest;
    if(!label_dict_get(lr->label, &dest)){
        assert_fail("UNKNOWN LABEL");
    }

    int offset;
    switch(lr->flag) {
        case 24:
            offset = (dest - lr->address - 8) >> 2;
            *out_word = 0x00FFFFFF & offset;
            return;
        default:
            assert("NOT IMPLEMENTED");
    }
}

static void asm_one(Emitter *emitter, const char *s) {

    if(follows_eof(s)) {
        /* blank line */
        return;
    }

    Substring one = {0};
    if(!parse_one(&s, &one)) {
        assert_fail("UNKNOWN ASSEMBLY");
    }

    if(one_is_label(&one)) {
        if(find_label_symbol(&one)) {
            assert_fail("DUPLICATE LABEL");
        }

        int label = to_label_symbol(&one);
        int address = emitter_current_address(emitter);
        label_dict_put(label, address);
        return;
    }

    int mnemonic = to_mnemonic_symbol(&one);
    if(mnemonic == RAW) {
        int word;
        if(parse_raw_word(&s, &word) && follows_eof(s)) {
            emitter_emit_word(emitter, word);
            return;;
        }
    } else if(B == mnemonic) {
        Substring subs = {0};
        if(parse_label(&s, &subs) && follows_eof(s)) {
            UnresolveAddress r = {
                .address = emitter_current_address(emitter),
                .label = to_label_symbol(&subs),
                .flag = 24,
            };
            unresolve_address_push(&r);
            emitter_emit_word(emitter, 0xEA000000);
            return;
        }
    } else if(LDR == mnemonic || STR == mnemonic) {
        int l_bit = str_eq_subs("ldr", &one)? 0x00100000: 0;

        int rd, rn;
        if(parse_register(&s, &rd)
            && skip_comma(&s)
            && skip_sbracket_open(&s)
            && parse_register(&s, &rn)) {

            if(follows_sbracket_close(s)) {
                if(skip_sbracket_close(&s)
                    && follows_eof(s)) {

                    emitter_emit_word(emitter, 0xE5800000 + l_bit + (rn << 16) + (rd << 12));
                    return;
                }
            } else {
                int offset;
                if(skip_comma(&s)
                    && parse_immediate(&s, &offset)
                    && skip_sbracket_close(&s)
                    && follows_eof(s)) {

                    int u_bit = (offset < 0)? 0: 0x00800000;
                    emitter_emit_word(emitter, 0xE5000000 + l_bit + u_bit + (rn << 16) + (rd << 12) + (abs(offset) & 0xFFF));
                    return;
                }
            }
        }
    } else if(MOV == mnemonic) {
        int rd;
        if(parse_register(&s, &rd)
            && skip_comma(&s)) {

            if(follows_register(s)) {
                int rm;
                if(parse_register(&s, &rm)
                    && follows_eof(s)) {
                    emitter_emit_word(emitter, 0xE1A00000 + (rd << 12) + rm);
                    return;
                }
            } else {
                int imm;
                if(parse_immediate(&s, &imm)
                    && follows_eof(s)) {
                    /* not implement 4bit rotate. only 8 bit immediate value is supported*/
                    if(imm < 0 || 0xFF < imm) {
                        assert_fail("INVALID REGISTER NUMBER");
                    }

                    emitter_emit_word(emitter, 0xE3A00000 + (rd << 12) + imm);
                    return;
                }
            }
        }
    }


    assert_fail("UNKNOWN ASSEMBLY");
}


static int string_to_mnemonic_symbol(const char *s) {
    Substring subs = {.str = s, .len = strlen(s)};
    return to_mnemonic_symbol(&subs);
}

static void prepare_mnemonic_symbol() {
    MOV = string_to_mnemonic_symbol("mov");
    RAW = string_to_mnemonic_symbol(".raw");
    LDR = string_to_mnemonic_symbol("ldr");
    STR = string_to_mnemonic_symbol("str");
    B = string_to_mnemonic_symbol("b");
}


/* unit test */
static void test_init() {
    mnemonic_symbol_clear();
    label_symbol_clear();
    label_dict_clear();
    unresolve_address_clear();
    prepare_mnemonic_symbol();
}

static void test_b_label() {
    test_init();

    char bin[10];
    Emitter *emitter = emitter_new(bin);

    asm_one(emitter, "b label");

    int address = emitter_current_address(emitter);
    int i = (bin[0] & 0xFF) == 0x00
        && (bin[1] & 0xFF) == 0x00
        && (bin[2] & 0xFF) == 0x00
        && (bin[3] & 0xFF) == 0xEA;

    assert(address == 4 && i);

    UnresolveAddress actual = {0};
    unresolve_address_pop(&actual);

    UnresolveAddress exp = {
        .address = 0,
        .label = 10001,
        .flag = 24};

    int eq = exp.address == actual.address
        && exp.label == actual.label
        && exp.flag == actual.flag;

    assert(eq);

}

static void test_label_address_reference() {
    unresolve_address_clear();
    UnresolveAddress lr = {.label = -1};
    unresolve_address_push(&lr);
    lr.label = 2;
    unresolve_address_push(&lr);

    unresolve_address_pop(&lr);
    assert(2 == lr.label);
    unresolve_address_pop(&lr);
    assert(-1 == lr.label);
}



void assembler_test() {

    test_b_label();

    test_label_address_reference();

    /*
    verify_asm_one_inst("mov r1, r2", 0xE1A01002);
    verify_asm_one_inst(" mov r15 ,  r0  ", 0xE1A0F000);
    verify_asm_one_inst("mov r1, #0x68", 0xE3A01068);

    verify_asm_one_inst(".raw 0x12345678", 0x12345678);
    verify_asm_one_inst(".raw 0x80000001", 0x80000001);

    verify_asm_one_inst("ldr r1, [r15, #0x30]", 0xE59F1030);
    verify_asm_one_inst("ldr r1, [r15, #-0x30]", 0xE51F1030);
    verify_asm_one_inst("ldr r1, [r15]", 0xE59F1000);
    verify_asm_one_inst("str r0, [r1]", 0xE5810000);
    */
}

