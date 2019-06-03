#include <string.h>
#include <assert.h>

#include "cl_utils.h"

static void assert_str_eq(const char *expect, const char *actual);
static void verify_print_asm_unknown(const int input);
static void verify_print_asm(const int input, const char *expect);


static int print_asm(int word) {
    if(0xE3A00000 == (0xE3A00000 & word)) {
        /* move */
        int tmp = word & 0x0000FFFF;
        int rd = tmp >> 12;
        int op2 = tmp & 0x0FFF;
        
        cl_printf("mov r%i, #%#04x\n", rd, op2);
        return 1;
    }

    return 0;
}

static void test_print_asm() {
    verify_print_asm_unknown(0x64646464);
    verify_print_asm(0xE3A01068, "mov r1, #0x68\n");
    verify_print_asm(0xE3A0106C, "mov r1, #0x6c\n");
    verify_print_asm(0xE3A0200A, "mov r2, #0x0a\n");
}

int main() {
    test_print_asm();
}


static void assert_str_eq(const char *expect, const char *actual) {
    int eq = strcmp(expect, actual) == 0;
    assert(eq);
}

static void verify_print_asm_unknown(const int input) {
    cl_enable_buffer_mode();
    cl_clear_output();

    int unknown = !print_asm(input);
    assert(unknown);
}

static void verify_print_asm(const int input, const char *expect) {
    cl_enable_buffer_mode();
    cl_clear_output();

    int success = print_asm(input);
    assert(success);

    char *actual = cl_get_result(0);
    assert_str_eq(expect, actual);
}

