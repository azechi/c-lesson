#include <assert.h>
#include <stdlib.h>
#include "cl_getline.h"
#include "parser.h"
#include "assembler_symbol_dict.h"
#include "assembler.h"
#include "assembler_emitter.h"
#include "assembler_label_dict.h"
#include "assembler_private.h"
#include "assembler_delay_emit_word_list.h"
#include "assembler_unresolve_address_list.h"

#include <string.h>

#define assert_fail(msg) assert(0&&(msg))


typedef enum UnresolveAddressType_ {
    ADDR_Imm_24,
    ADDR_Imm_12,
    ADDR_CONSTANT
} UnresolveAddressType;

int MOV;
int ADD;
int CMP;
int RAW;
int LDR;
int LDRB;
int STR;
int B;
int BL;
int BNE;
int STMDB;
int LDMIA;

static void prepare_mnemonic_symbol();
static void asm_one(Emitter *emitter, const char *s);
static void asm_address(Emitter *emitter, UnresolveAddress *lr);

int assemble(char *const bin) {
    Emitter *emitter = emitter_new(bin);
    prepare_mnemonic_symbol();
    char *str;
    while(cl_getline(&str) >= 0) {
        asm_one(emitter, str);
    }

    DelayEmitWord ew;
    while(delay_emit_word_pop(&ew)) {
        int address = emitter_current_address(emitter);
        label_dict_put(ew.label, address);

        switch(ew.type) {
            case POOLITEM_CONSTANT:
                emitter_emit_word(emitter, ew.u.constant);
                break;
            case POOLITEM_LABEL_REFERENCE:
                {
                    UnresolveAddress ula = {
                        .address = address,
                        .label = ew.u.target_label,
                        .flag = ADDR_CONSTANT
                    };
                    unresolve_address_push(&ula);
                    emitter_emit_word(emitter, 0);
                }
                break;
            default:
                assert_fail("INVALID LITERALPOOLITEMTYPE VALUE");
        }
    }

    UnresolveAddress lr;
    while(unresolve_address_pop(&lr)) {
        asm_address(emitter ,&lr);
    }

    int len = emitter_current_address(emitter);
    emitter_free(emitter);
    return len;
}


static void asm_address(Emitter *emitter, UnresolveAddress *lr) {
    int dest;
    if(!label_dict_get(lr->label, &dest)){
        assert_fail("UNKNOWN LABEL");
    }

    int offset;
    switch(lr->flag) {
        case ADDR_Imm_24:
            offset = (dest - lr->address - 8) >> 2;
            emitter_patch_word(emitter, lr->address, 0x00FFFFFF & offset);
            return;
        case ADDR_Imm_12:
            {
                /* sign bit */
                offset = (dest - lr->address - 8);
                int u_bit = (offset < 0)? 0: 0x00800000;
                emitter_patch_word(emitter, lr->address, (0x00000FFF & offset) + u_bit);
            }
            return;
        case ADDR_CONSTANT:
            {
                offset = 0x00010000;
                emitter_patch_word(emitter, lr->address, dest + offset);
            }
            return;
        default:
            assert("NOT IMPLEMENTED");
            return;
    }
}

static void asm_data_processing(Emitter *emitter, int mnemonic, const char *s) {
    if(LDR == mnemonic || STR == mnemonic || LDRB == mnemonic) {
        int l_bit = (LDR == mnemonic || LDRB == mnemonic)? 0x00100000: 0;
        int b_bit = (LDRB == mnemonic)? 0x00400000: 0;


        /*
            ldr rd, [rn]        // [rn, #0x00]
            ldr rd, [rn, #0xFFF]
            ldr rd, [rn, #-0xFFF]
            ldr rd, =0x01234567 // [r15, #0x0F]
            ldr rd, =label      // [r15, #-0xFF]
        */

        int rd;
        parse_register(&s, &rd);
        skip_comma(&s);

        if(follows_sbracket_open(s)) {
            int rn;
            skip_sbracket_open(&s);
            parse_register(&s, &rn);

            if(follows_sbracket_close(s)) {
                skip_sbracket_close(&s);
                eof(&s);
                int u_bit = 0x00800000;
                emitter_emit_word(emitter, 0xE5000000 + l_bit + b_bit + u_bit + (rn << 16) + (rd << 12));
                return;
            }
            skip_comma(&s);

            int offset;
            parse_immediate(&s, &offset);
            skip_sbracket_close(&s);
            eof(&s);

            int u_bit = (offset < 0)? 0: 0x00800000;
            emitter_emit_word(emitter, 0xE5000000 + l_bit + b_bit + u_bit + (rn << 16) + (rd << 12) + (abs(offset) & 0xFFF));
            return;
        }

        skip_equal_sign(&s);

        if(follows_raw_word(s)) {
            const char *begin = s - 1; /* -1 = '=' */
            int word;
            parse_raw_word(&s, &word);

            Substring implicit_label = {.str = begin, .len = s - begin};

            UnresolveAddress r = {
                .address = emitter_current_address(emitter),
                .label = to_label_symbol(&implicit_label),
                .flag = ADDR_Imm_12,
            };
            unresolve_address_push(&r);

            DelayEmitWord item = {
                .label = to_label_symbol(&implicit_label),
                .type = POOLITEM_CONSTANT,
                .u.constant = word
            };
            delay_emit_word_push(&item);

        } else {
            const char *begin = s - 1;
            Substring target_label = {0};
            parse_one(&s, &target_label);

            Substring implicit_label = {.str = begin, .len = s - begin};

            UnresolveAddress r = {
                .address = emitter_current_address(emitter),
                .label = to_label_symbol(&implicit_label),
                .flag = ADDR_Imm_12,
            };

            unresolve_address_push(&r);

            DelayEmitWord item = {
                .label = to_label_symbol(&implicit_label),
                .type = POOLITEM_LABEL_REFERENCE,
                .u.target_label = to_label_symbol(&target_label)
            };
            delay_emit_word_push(&item);
        }
        eof(&s);

        /* append pseudo label reference */

        /* delay(register pseudo label address) */


        int rn = 15;
        emitter_emit_word(emitter, 0xE5800000 + l_bit + b_bit + (rn << 16) + (rd << 12));
        return;
    } else if(CMP == mnemonic) {
        /* cmp rn, #0xFFF */
        int rn;
        parse_register(&s, &rn);
        skip_comma(&s);
        int imm;
        parse_immediate(&s, &imm);
        eof(&s);

        int i_bit = 0x02000000;
        int s_bit = 0x00100000;
        int opcode = 0x01400000;

        /* not implement 4bit rotate. only 8 bit immediate value is supported*/
        if(imm < 0 || 0xFF < imm) {
            assert_fail("INVALID REGISTER NUMBER");
        }

        emitter_emit_word(emitter, 0xE0000000 + s_bit + i_bit + opcode + (rn << 16) + imm);
        return;


    } else if(ADD == mnemonic) {
        /* add rd, rn, #0xFFF */
        int rd;
        parse_register(&s, &rd);
        skip_comma(&s);
        int rn;
        parse_register(&s, &rn);
        skip_comma(&s);
        int imm;
        parse_immediate(&s, &imm);
        eof(&s);

        int i_bit = 0x02000000;
        int opcode = 0x00800000;

        emitter_emit_word(emitter, 0xE0000000 + i_bit + opcode + (rd << 16) + (rn << 12) + imm);
        return;


    } else if(MOV == mnemonic) {
        int rd;
        parse_register(&s, &rd);
        skip_comma(&s);
        /* mov rd, */

        if(follows_register(s)) {
            /* mov rd, rm */
            int rm;
            parse_register(&s, &rm);
            eof(&s);

            emitter_emit_word(emitter, 0xE1A00000 + (rd << 12) + rm);
            return;
        }

        /* mov rd, #0xFFF; */
        int imm;
        parse_immediate(&s, &imm);
        eof(&s);

        /* not implement 4bit rotate. only 8 bit immediate value is supported*/
        if(imm < 0 || 0xFF < imm) {
            assert_fail("INVALID REGISTER NUMBER");
        }

        emitter_emit_word(emitter, 0xE3A00000 + (rd << 12) + imm);
        return;
    }

    assert_fail("NOT IMPLEMENTED");
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
        int label = to_label_symbol(&one);
        int address = emitter_current_address(emitter);
        label_dict_put(label, address);
        return;
    }

    int mnemonic = to_mnemonic_symbol(&one);
    if(mnemonic == RAW) {
        if(follows_raw_word(s)) {
            int word;
            parse_raw_word(&s, &word);
            eof(&s);
            emitter_emit_word(emitter, word);
            return;
        }

        char s_buf[1024];
        parse_raw_string(&s, s_buf);
        eof(&s);
        emitter_emit_string(emitter, s_buf);
        return;
    } else if(B == mnemonic || BL == mnemonic || BNE == mnemonic) {
        int cond = (BNE == mnemonic)? 0x10000000: 0xE0000000;

        int l_bit = (BL == mnemonic)? 0x01000000: 0;

        Substring subs = {0};
        if(parse_one(&s, &subs) && follows_eof(s)) {
            UnresolveAddress r = {
                .address = emitter_current_address(emitter),
                .label = to_label_symbol(&subs),
                .flag = ADDR_Imm_24,
            };
            unresolve_address_push(&r);
            emitter_emit_word(emitter, cond + 0x0A000000 + l_bit);
            return;
        }

    } else if (STMDB == mnemonic || LDMIA == mnemonic) {
        /* Block Data Transfer */

        int cond = 0xE0000000;
        int p_bit = (STMDB == mnemonic)? 0x01000000: 0;
        int u_bit = (LDMIA == mnemonic)? 0x00800000: 0;
        int s_bit = 0;
        int l_bit = (LDMIA == mnemonic)? 0x00100000: 0;


        int rn;
        parse_register(&s, &rn);
        int w_bit =  try_skip_bang(&s)? 0x00200000: 0;
        skip_comma(&s);

        int reg_list;
        parse_register_list(&s, &reg_list);
        emitter_emit_word(emitter, cond + 0x08000000 + p_bit + u_bit + s_bit + w_bit + l_bit + (rn << 16) + reg_list);
        return;

    } else {
        asm_data_processing(emitter, mnemonic, s);
        return;
    }

    assert_fail("UNKNOWN ASSEMBLY");
}


static int string_to_mnemonic_symbol(const char *s) {
    Substring subs = {.str = s, .len = strlen(s)};
    return to_mnemonic_symbol(&subs);
}

static void prepare_mnemonic_symbol() {
    MOV = string_to_mnemonic_symbol("mov");
    ADD = string_to_mnemonic_symbol("add");
    CMP = string_to_mnemonic_symbol("cmp");
    RAW = string_to_mnemonic_symbol(".raw");
    LDR = string_to_mnemonic_symbol("ldr");
    LDRB = string_to_mnemonic_symbol("ldrb");
    STR = string_to_mnemonic_symbol("str");
    B = string_to_mnemonic_symbol("b");
    BL = string_to_mnemonic_symbol("bl");
    BNE = string_to_mnemonic_symbol("bne");
    STMDB = string_to_mnemonic_symbol("stmdb");
    LDMIA = string_to_mnemonic_symbol("ldmia");
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
        .flag = ADDR_Imm_24
    };

    int eq = exp.address == actual.address
        && exp.label == actual.label
        && exp.flag == actual.flag;

    assert(eq);

}

static void test_label_address_reference() {
    unresolve_address_clear();
    UnresolveAddress lr = {.label = 1};
    unresolve_address_push(&lr);
    lr.label = 2;
    unresolve_address_push(&lr);
    lr.label = 3;
    unresolve_address_push(&lr);

    UnresolveAddress actual = {0};
    unresolve_address_pop(&actual);
    assert(3 == actual.label);
    unresolve_address_pop(&actual);
    assert(2 == actual.label);
    unresolve_address_pop(&actual);
    assert(1 == actual.label);
}

static void test_delay_emit_word() {
    delay_emit_word_clear();
    DelayEmitWord item = {.label = 1};
    delay_emit_word_push(&item);
    item.label = 2;
    delay_emit_word_push(&item);
    item.label = 3;
    delay_emit_word_push(&item);

    DelayEmitWord actual = {0};
    delay_emit_word_pop(&actual);
    assert(1 == actual.label);
    delay_emit_word_pop(&actual);
    assert(2 == actual.label);
    delay_emit_word_pop(&actual);
    assert(3 == actual.label);

    int notany = !delay_emit_word_pop(&actual);
    assert(notany);


}

void test_asm_one_raw_string() {
    char *input = ".raw \"hello, world\n\"";
    char expect[] = {
        0X68, 0X65, 0X6C, 0X6C,
        0X6F, 0X2C, 0X20, 0X77,
        0X6F, 0X72, 0X6C, 0X64,
        0X0A, 0X00, 0X00, 0X00
    };

    test_init();

    char bin[100 * 1024];
    Emitter *emitter = emitter_new(bin);

    asm_one(emitter, input);

    int address = emitter_current_address(emitter);
    assert(address == sizeof(expect));

    int i = 0;
    int eq = 1;
    for(; eq && i < sizeof(expect); i++) {
        eq = expect[i] == bin[i];
    }
    assert(eq);
}

static void test_bl() {

    test_init();

    char bin[10];
    Emitter *emitter = emitter_new(bin);

    asm_one(emitter, "bl label");

    int address = emitter_current_address(emitter);
    int i = (bin[0] & 0xFF) == 0x00
        && (bin[1] & 0xFF) == 0x00
        && (bin[2] & 0xFF) == 0x00
        && (bin[3] & 0xFF) == 0xEB;

    assert(address == 4 && i);

    UnresolveAddress actual = {0};
    unresolve_address_pop(&actual);

    UnresolveAddress exp = {
        .address = 0,
        .label = 10001,
        .flag = ADDR_Imm_24
    };

    int eq = exp.address == actual.address
        && exp.label == actual.label
        && exp.flag == actual.flag;

    assert(eq);
}

void verify_asm_one(char *input, int expect) {
    test_init();
    char bin[10];
    Emitter *emitter = emitter_new(bin);

    asm_one(emitter, input);

    int address = emitter_current_address(emitter);
    int eq = (bin[0] & 0xFF) == (expect & 0xFF)
        && (bin[1] & 0xFF) == (expect >> 8 & 0xFF)
        && (bin[2] & 0xFF) == (expect >> 16 & 0xFF)
        && (bin[3] & 0xFF) == (expect >> 24 & 0xFF);

    assert(address == 4 && eq);
}

void assembler_test() {

    test_b_label();

    test_label_address_reference();
    test_delay_emit_word();

    test_asm_one_raw_string();

    test_bl();

    verify_asm_one("mov r1, r2", 0xE1A01002);
    verify_asm_one(" mov r15 ,  r0  ", 0xE1A0F000);
    verify_asm_one("mov r1, #0x68", 0xE3A01068);

    verify_asm_one(".raw 0x12345678", 0x12345678);
    verify_asm_one(".raw 0x80000001", 0x80000001);

    verify_asm_one("ldr r1, [r15, #0x30]", 0xE59F1030);
    verify_asm_one("ldr r1, [r15, #-0x30]", 0xE51F1030);
    verify_asm_one("ldr r1, [r15]", 0xE59F1000);
    verify_asm_one("str r0, [r1]", 0xE5810000);
    verify_asm_one("stmdb r13!, {r1, r14}", 0xE92D4002);
    verify_asm_one("ldmia r13!, {r1, r14}", 0xE8BD4002);


/*
	"ldr r0, =0x101f1000", 0xE59F0000, append(symbol("101F1000"), value = 0xE59F000), unresolve(address = 0, symbol("101F1000"))
	"ldr r0, =message", 0xE59F0000, unresolve(address = 0, symbol("message"))
*/

}

