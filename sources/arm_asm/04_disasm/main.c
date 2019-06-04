#include <stdint.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>

#include "cl_utils.h"

static void assert_str_eq(const char *expect, const char *actual);
static void verify_print_asm_unknown(uint32_t input);
static void verify_print_asm(uint32_t input, const char *expect);


static int print_asm(uint32_t word) {
    if(0xE2811001 == word) {
        /* add */
        cl_printfn("add r1, r1, #0x01");
        return 1;
    }

    if(0xE3530000 == word) {
        /* cmp */
        cl_printfn("cmp r3, #0x00");
        return 1;
    }

    if(0xE3A00000 == (0xE3A00000 & word)) {
        /* move */
        int tmp = word & 0x0000FFFF;
        int rd = tmp >> 12;
        int op2 = tmp & 0x0FFF;

        cl_printfn("mov r%i, #0x%02X", rd, op2);
        return 1;
    }

    if(0x0A000000 == (0x0A000000 & word)) {
        /* branch */
        int offset = 0x00FFFFFF & word;

        switch (word >> 28) {
            case 0x1: /* bne */
                if(offset != 0xFFFFFA) {
                    return 0;
                }
                cl_printfn("bne [r15, #-0x18]");
                return 1;
            case 0xE: /* b (eq) */
                if(offset != 0xFFFFFE) {
                    return 0;
                }
                cl_printfn("b [r15, #-0x08]");
                return 1;
            default:
                return 0;
        }
    }

    if(0xE5900000 == (0xE5900000 & word)) {
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

    if(0xE5800000 == (0xE5800000 & word)) {
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

    return 0;
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

