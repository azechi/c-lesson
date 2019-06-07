#include <stdint.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "cl_utils.h"

static void assert_str_eq(const char *expect, const char *actual);
static void verify_print_asm_unknown(int input);
static void verify_print_asm(int input, const char *expect);

static int print_asm_data_processing(int word);
static int print_asm_single_data_transfer(int word);
static int print_asm_block_data_transfer(int word);
static int print_asm_branch(int word);


static int print_asm(int word) {
    if((word & 0x0C000000) == 0x00000000) {
        return print_asm_data_processing(word);
    }

    if((word & 0x0C000000) == 0x04000000) {
        return print_asm_single_data_transfer(word);
    }

    if((word & 0x0E000000) == 0x08000000) {
        return print_asm_block_data_transfer(word);
    }

    if((word & 0x0E000000) == 0x0A000000) {
        return print_asm_branch(word);
    }

    return 0;

}

static int print_asm_data_processing(int word) {
    if((word & 0xF0000000) != 0xE0000000 ) {
        return 0;
    }

    int rn = word >> 16 & 0x0F;
    int rd = word >> 12 & 0x0F;

    char *s_op;
    char is_mov = 0;
    char is_cmp = 0;
    switch(word & 0x01F00000) {
        /* operation code + set condition code */
        case 0x00000000:
            s_op = "and";
            break;
        case 0x00800000:
            s_op = "add";
            break;
        case 0x00400000:
            s_op = "sub";
            break;
        case 0x01A00000:
            s_op = "mov";
            is_mov = 1;
            break;
        case 0x01500000:
            s_op = "cmp";
            is_cmp = 1;
            break;
        default:
            return 0;
    }

    if(word & 0x02000000) {
        /* operand2 is an immediate value */
        char rotate = word >> 8 & 0x0F;
        int imm = word & 0xFF;
        int val;
        if(rotate) {
            if((word & 0xFFF) == 0x302) {
                val = 0x08000000;
            } else {
                return 0;
            }
        } else {
            val = imm;
        }

        if(is_cmp) {
            cl_printfn("cmp r%i, #0x%02X", rn, val);
        } else if(is_mov) {
            cl_printfn("mov r%i, #0x%02X", rd, val);
        } else {
            cl_printfn("%s r%i, r%i, #0x%02X", s_op, rd, rn, val);
        }
        return 1;
    } else {
        /* operand2 is a register */
        int shift = word >> 4 & 0xFF;
        char rm = word & 0x0F;

        if((word & 0x00000FF0) == 0) {
            cl_printfn("mov r%i, r%i", rd, rm);
            return 1;
        }

        if(!(shift & 0x01)) {
            /* shift amount immediate  */
            return 0;
        }

       char rs = word >> 8 & 0x0F;
       switch(word & 0x000000F0) {
            case 0x30:
                /* lsr */
               cl_printfn("lsr r%i, r%i, r%i", rd, rm, rs);
               return 1;
            default:
               return 0;
       }
    }
}

static int print_asm_single_data_transfer(int word) {
    if((word & 0xF0000000) != 0xE0000000 ) {
        return 0;
    }

    int rn = word >> 16 & 0x0F;
    int rd = word >> 12 & 0x0F;
    int offset = word & 0xFFF;

    char *s_op;
    switch(word & 0x0FF00000) {
        case 0x05800000:
            s_op = "str";
            break;
        case 0x05900000:
            s_op = "ldr";
            break;
        case 0x05D00000:
            s_op = "ldrb";
            break;
        default:
            return 0;
    }

    char *s_sign = (offset < 0)? "-": "";

    if(offset) {
        cl_printfn("%s r%i, [r%i, #%s0x%02X]", s_op, rd, rn, s_sign, offset);
    } else {
        cl_printfn("%s r%i, [r%i]", s_op, rd, rn);
    }
    return 1;
}

static int print_asm_block_data_transfer(int word) {
    if((word & 0xF0000000) != 0xE0000000 ) {
        return 0;
    }

    char *s_op;
    char *s_writeback;
    switch(word >> 20 & 0x0FF) {
        case 0x92:
            s_op = "stmdb";
            s_writeback = "!";
            break;
        case 0x8B:
            s_op = "ldmia";
            s_writeback = "!";
            break;
        default:
            return 0;
    }

    char rn = word >> 16 & 0x0F;
    char *s_register_list;
    switch(word & 0x0000FFFF) {
        case 0x4002:
            s_register_list = "{r1, r14}";
            break;
        case 0x4006:
            s_register_list = "{r1, r2, r14}";
            break;
        default:
            return 0;
    }

    cl_printfn("%s r%i%s, %s", s_op, rn, s_writeback, s_register_list);
    return 1;
}

static int print_asm_branch(int word) {

    char *s_cond;
    switch(word & 0xF0000000) {
        case 0xE0000000:
            s_cond = "";
            break;
        case 0xD0000000:
            s_cond = "le";
            break;
        case 0x10000000:
            s_cond = "ne";
            break;
        default:
            return 0;
    }

    char *s_link = (word & 0x01000000)? "l": "";

    int offset =(int)(word << 8) >> 6;
    char *s_sign = (offset < 0)? "-": "";

    cl_printfn("b%s%s [r15, #%s0x%02X]", s_link, s_cond, s_sign ,abs(offset));
    return 1;
}


static void test_print_asm() {
    verify_print_asm_unknown(0x64646464);

    verify_print_asm(0xE3A01068, "mov r1, #0x68\n");
    verify_print_asm(0xE3A0106C, "mov r1, #0x6C\n");
    verify_print_asm(0xE3A0200A, "mov r2, #0x0A\n");
    verify_print_asm(0xE1A0F00E, "mov r15, r14\n");

    verify_print_asm(0xEAFFFFFE, "b [r15, #-0x08]\n");
    verify_print_asm(0x1AFFFFFA, "bne [r15, #-0x18]\n");

    verify_print_asm(0xE59F0038, "ldr r0, [r15, #0x38]\n");
    verify_print_asm(0xE59F002C, "ldr r0, [r15, #0x2C]\n");
    verify_print_asm(0xE59F102C, "ldr r1, [r15, #0x2C]\n");

    verify_print_asm(0xE5D13000, "ldrb r3, [r1]\n");

    verify_print_asm(0xE5801000, "str r1, [r0]\n");
    verify_print_asm(0xE5802000, "str r2, [r0]\n");

    verify_print_asm(0xE2811001, "add r1, r1, #0x01\n");

    verify_print_asm(0xE3530000, "cmp r3, #0x00\n");

    verify_print_asm(0xE2422004, "sub r2, r2, #0x04\n");

    verify_print_asm(0xE1A03231, "lsr r3, r1, r2\n");
    verify_print_asm(0xE203300F, "and r3, r3, #0x0F\n");

    verify_print_asm(0xE3A0D302, "mov r13, #0x8000000\n");

    verify_print_asm(0xE92D4002, "stmdb r13!, {r1, r14}\n");
}


int main(int argc, char *argv[]) {

    if(argc <= 1) {
        test_print_asm();
        return 0;
    }

    FILE *f = NULL;
    f = fopen(argv[1], "rb");
    if(!f) {
        printf("ERROR! FILE CANNOT OPEN: [%s]", argv[1]);
        return 1;
    }

    int count = 0x00010000;
    int word;
    int unknown = 0;
    /* ファイルサイズが必ず4byteで割り切れるとする */
    /* hostはリトルエンディアンとする */
    while(fread(&word, 4, 1, f)) {
        cl_printf("0x%08X ", count);
        if(unknown || (unknown = !print_asm(word))){
            uint8_t *h = (uint8_t *)&word;
            cl_printfn("%02X %02X %02X %02X", h[0], h[1], h[2], h[3]);
        }

        count += 4;
    }

}


static void assert_str_eq(const char *expect, const char *actual) {
    int eq = strcmp(expect, actual) == 0;
    assert(eq);
}

static void verify_print_asm_unknown(int input) {
    cl_enable_buffer_mode();
    cl_clear_output();

    int unknown = !print_asm(input);
    assert(unknown);
}

static void verify_print_asm(int input, const char *expect) {
    cl_enable_buffer_mode();
    cl_clear_output();

    int success = print_asm(input);
    assert(success);

    char *actual = cl_get_result(0);
    assert_str_eq(expect, actual);
}

