#include <assert.h>
#include <stdlib.h>
#include "cl_getline.h"
#include "parser.h"
#include "symbol.h"
#include "label.h"
#include "assembler.h"


#define assert_fail(msg) assert(0&&(msg))


typedef struct LableAddressReference_ {
    int address;
    int label;
    int flag;
    struct LableAddressReference_ *next;
} LabelAddressReference;

static LabelAddressReference *head = NULL;


static void label_address_reference_push(LabelAddressReference *r) {
    LabelAddressReference *h = head;
    while(h) {
        h = h->next;
    }

    h = malloc(sizeof(*head));
    *h = *r;
    h->next = head;
    head = h;
}

static int label_address_reference_pop(LabelAddressReference *out_r) {
    if(!head) {
        return 0;
    }

    *out_r = *head;

    LabelAddressReference *tmp = head;
    head = head->next;
    free(tmp);;
    return 1;
}

static int asm_one(const char *s, int address, int *out_word);
static void asm_address(LabelAddressReference *lr, int *out_word);
static void emit_word(Emitter *emitter, int word);
static void patch_word(Emitter *emitter, int address ,int word);

int assemble(Emitter *emitter) {
    char *str;
    while(cl_getline(&str) >= 0) {
        int word = 0;
        if(asm_one(str, emitter->pos, &word)) {
            emit_word(emitter, word);
        }
    }

    LabelAddressReference lr;
    while(label_address_reference_pop(&lr)) {
        int word = 0;
        asm_address(&lr, &word);
        patch_word(emitter, lr.address, word);
    }

    return 1;
}

static void asm_address(LabelAddressReference *lr, int *out_word) {
    int dest;
    if(!label_dict_get(lr->label, &dest)){
        assert_fail("UNKNOWN LABEL");
    }

    int offset;
    switch(lr->flag) {
        case 24:
            offset = dest - lr->address - 8;
            *out_word = 0x00FFFFFF & offset;
            return;
        default:
            assert("NOT IMPLEMENTED");
    }
}

static int asm_one(const char *s, int address, int *out_word) {

    if(follows_eof(s)) {
        /* blank line */
        return 0;
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
        label_dict_put(label, address);
        return 0;
    }

    int mnemonic = to_mnemonic_symbol(&one);
    if(mnemonic == mnemonic_raw) {
        int word;
        if(parse_raw_word(&s, &word) && follows_eof(s)) {
            *out_word = word;
            return 1;
        }
    } else if(mnemonic_b == mnemonic) {
        Substring subs = {0};
        if(parse_label(&s, &subs) && follows_eof(s)) {
            LabelAddressReference r = {
                .address = address,
                .label = to_label_symbol(&subs),
                .flag = 24,
            };
            label_address_reference_push(&r);
            *out_word = 0xEA000000;
            return 1;
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
                    return 1;
                }
            } else {
                int offset;
                if(skip_comma(&s)
                    && parse_immediate(&s, &offset)
                    && skip_sbracket_close(&s)
                    && follows_eof(s)) {

                    int u_bit = (offset < 0)? 0: 0x00800000;
                    *out_word = 0xE5000000 + l_bit + u_bit + (rn << 16) + (rd << 12) + (abs(offset) & 0xFFF);
                    return 1;
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
                    return 1;
                }
            } else {
                int imm;
                if(parse_immediate(&s, &imm)
                    && follows_eof(s)) {
                    /* not implement 4bit rotate. only 8 bit immediate value is supported*/
                    if(imm < 0 || 0xFF < imm) {
                        assert_fail("INVALID REGISTER NUMBER");
                    }

                    *out_word = 0xE3A00000 + (rd << 12) + imm;
                    return 1;
                }
            }
        }
    }


    assert_fail("UNKNOWN ASSEMBLY");
}

static void emit_word(Emitter *emitter, int word) {
    emitter->buf[emitter->pos++] = word & 0xFF;
    emitter->buf[emitter->pos++] = word >> 8 & 0xFF;
    emitter->buf[emitter->pos++] = word >> 16 & 0xFF;
    emitter->buf[emitter->pos++] = word >> 24 & 0xFF;
}

static void patch_word(Emitter *emitter, int address ,int word) {
    emitter->buf[address++] |= word & 0xFF;
    emitter->buf[address++] |= word >> 8 & 0xFF;
    emitter->buf[address++] |= word >> 16 & 0xFF;
    emitter->buf[address++] |= word >> 24 & 0xFF;
}


/* unit test */
static void test_b_label() {
    char *input = "b label";
    int expect_word = 0xEA000000;
    LabelAddressReference expect_ref= {
        .address = 0,
        .label = 10001,
        .flag = 24
    };

    int word;
    int success = asm_one(input, 0, &word);
    assert(success);
    assert(expect_word == word);

    LabelAddressReference actual = *head;
    int eq = expect_ref.address == actual.address
        && expect_ref.label == actual.label
        && expect_ref.flag == actual.flag;

    assert(eq);
}

void test_label_address_reference() {
    head = NULL;
    LabelAddressReference lr = {.label = -1};
    label_address_reference_push(&lr);
    lr.label = 2;
    label_address_reference_push(&lr);

    label_address_reference_pop(&lr);
    assert(2 == lr.label);
    label_address_reference_pop(&lr);
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

