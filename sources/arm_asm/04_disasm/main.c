#include <stdint.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "cl_utils.h"

static void assert_str_eq(const char *expect, const char *actual);
static void verify_print_asm_unknown(uint32_t input);
static void verify_print_asm(uint32_t input, const char *expect);


static int print_asm(uint32_t word) {
    if(0xE2400000 == (0xFFF00000 & word)) {
        /* sub */
        int r_op1 = (word >> 16) & 0xF;
        int r_dest = (word >> 12) & 0xF;

        int rotate = (word >> 4) & 0xF;
        if(rotate != 0) {
            return 0;
        }
        int imm = word & 0xFF;
        cl_printfn("sub r%i, r%i, #0x%02X", r_dest, r_op1, imm);
        return 1;
    }

    if(0xE5900000 == (0xFF900000 & word)) {
        /* ldr */
        uint32_t tmp = word & 0x000FFFFF;
        uint32_t rn = tmp >> 16;
        uint32_t rd = (tmp & 0x0FFFF) >> 12;
        uint32_t offset = tmp & 0xFFF;

        uint32_t b = (word >> 20) & 0x004; /* byte/word */
        char *op = b ? "ldrb": "ldr";

        if(offset == 0){
            cl_printfn("%s r%i, [r%i]", op, rd, rn);
        }else {
            cl_printfn("%s r%i, [r%i, #0x%02X]", op, rd, rn, offset);
        }
        return 1;
    }

    if(0xE5800000 == (0xFF800000 & word)) {
        /* str */
        int tmp = word & 0x000FFFFF;
        int rn = tmp >> 16; /* base register */
        int rd = (tmp & 0x0FFFF) >> 12; /* source/destination register */
        int offset = tmp & 0xFFF;

        if(offset == 0) {
            cl_printfn("str r%i, [r%i]", rd, rn);
            return 1;
        }
    }

    char cond = word >> 28 & 0xF;
    int instruction = word >> 20 & 0x0FF; /* instruction is always positive */
    int rest = word & 0x000FFFFF; /* rest is always positive */
    switch (instruction >> 6) {
        case 0:
            /* 00 data processing, multiply, sigle data swap */
            if(cond != 0xE && instruction & 0x1) {
                /* set condition codes = 1 */
                return 0;
            }

            char imm = instruction >> 5;
            char opcode = instruction >> 1 & 0x0F;
            char rn = rest >> 16;
            char rd = rest >> 12 & 0x0F;
            int operand2 = rest & 0x00FFF;
            switch(opcode) {
                case 0xD: /* 1101 MOV {MOV, LSR} */
                    if(rn) {
                        return 0;
                    }

                    if(imm == 0) {
                        char rm = operand2 & 0x00F;
                        int shift = operand2 >> 4;
                        if(shift == 0) {
                            /* MOV */
                            cl_printfn("mov r%i, r%i", rd, rm);
                            return 1;
                        }

                        /* LSR */
                        if(!rn
                                && (operand2 >> 4 & 0x09) != 0x1
                                && (operand2 >> 5 & 0x01) != 0x1) {
                            return 0;
                        }
                        char rs = operand2 >> 8;
                        cl_printfn("lsr r3, r1, r2", rd, rm, rs);
                        return 1;
                    } else {
                        /* immediate = 1 */
                        char rotate = operand2 >> 8;
                        int val = operand2 & 0x0FF;
                        if(rotate) {
                            return 0;
                        }
                        cl_printfn("mov r%i, #0x%02X", rd, val);
                        return 1;
                    }
                    return 0;
                case 0x0: /* 0000 AND */
                    {
                        if(!imm) {
                            return 0;
                        }
                        char rotate = operand2 >> 8;
                        int val = operand2 & 0x0FF;
                        if(rotate) {
                            return 0;
                        }

                        cl_printfn("and r%i, r%i, #0x%02X", rd, rn, val);
                        return 1;
                    }
                case 0xA: /* 1010 CMP  */
                    {
                        if(!imm && !(instruction & 0x1)){
                            return 0;
                        }
                        char rotate = operand2 >> 8;
                        int val = operand2 & 0xFF;
                        if(rotate) {
                            return 0;
                        }

                        cl_printfn("cmp r%i, #0x%02X", rn, val);
                        return 1;
                    }
                case 0x4: /* 0100 ADD */
                    {
                        if(!imm && !(instruction & 0x1)){
                            return 0;
                        }
                        char rotate = operand2 >> 8;
                        int val = operand2 & 0xFF;
                        if(rotate) {
                            return 0;
                        }

                        cl_printfn("add r%i, r%i, #0x%02X", rd, rn, val);
                        return 1;
                    }
            }
            return 0;
        case 1: /* 01 */
            return 0;
        case 2: /* 10 Block Data Transfer, Branch */
            {
                char *s_link = (word >> 24 & 0x01)? "l": "";

                if((word >> 25 & 0x07) == 0x05) {
                    /* branch L = 0 */
                    int offset =(int)(word << 8) >> 6;
                    char *s_cond;
                    switch(cond) {
                        case 0xE:
                            s_cond = "";
                            break;
                        case 0xD:
                            s_cond = "le";
                            break;
                        case 0x01:
                            s_cond = "ne";
                            break;
                        default:
                            return 0;
                    }

                    char s_offset[20];
                    sprintf(s_offset,  offset < 0? "#-0x%02X": "#0x%02X", abs(offset));
                    cl_printfn("b%s%s [r15, %s]", s_link, s_cond, s_offset);
                    return 1;
                }
            }
            return 0;
        case 3: /* 11 */
        default:
            return 0;
    }
}

static void test_print_asm() {
    verify_print_asm_unknown(0x64646464);

    verify_print_asm(0xE3A01068, "mov r1, #0x68\n");
    verify_print_asm(0xE3A0106C, "mov r1, #0x6C\n");
    verify_print_asm(0xE3A0200A, "mov r2, #0x0A\n");

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
    uint32_t word;
    int unknown = 0;
    /* ファイルサイズが必ず4byteで割り切れるとする */
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

static void verify_print_asm_unknown(uint32_t input) {
    cl_enable_buffer_mode();
    cl_clear_output();

    int unknown = !print_asm(input);
    assert(unknown);
}

static void verify_print_asm(uint32_t input, const char *expect) {
    cl_enable_buffer_mode();
    cl_clear_output();

    int success = print_asm(input);
    assert(success);

    char *actual = cl_get_result(0);
    assert_str_eq(expect, actual);
}

