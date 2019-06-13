#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "cl_getline.h"

#define BUF_SIZE 80
static char buf[BUF_SIZE];
static FILE *fp = NULL;
static const char *str = NULL;


void cl_getline_set_str(const char *s) {
    str = s;
}

int cl_getline_set_file(const char *filename) {
    fp = fopen(filename, "r");
    if(!fp) {
        printf("ERROR! FILE CANNOT OPEN: [%s]", filename);
        return 0;
    }
    return 1;
}

int cl_getline(char **out_buf) {

    if(fp) {
        if(fgets(buf, BUF_SIZE, fp)) {
            char *p = strchr(buf, '\n');
            if(p) {
                *p = '\0';
            }

            *out_buf = buf;
            return strlen(buf);
        }
    } else if (str) {
        if(*str != '\0') {
            int i = 0;
            for(; str[i] != '\n' && str[i] != '\0'; i++);
            ;
            strncpy(buf, str, i);
            buf[i] = '\0';

            str += i + 1;

            *out_buf = buf;
            return i;
        }
    }

    return EOF;
}


/* unit test */
static void assert_cl_getline_eof() {
    int actual = cl_getline(NULL);
    assert(EOF == actual);
}

static void assert_cl_getline(const char *expect) {
    int expect_len = strlen(expect);

    int actual_len;
    char *actual;
    actual_len = cl_getline(&actual);
    assert(expect_len == actual_len);

    int eq = strcmp(expect, actual) == 0;
    assert(eq);
}

static void test_1() {
    char *input = "a" "\n" "b" "\n";
    cl_getline_set_str(input);
    assert_cl_getline("a");
    assert_cl_getline("b");
    assert_cl_getline_eof();
}

static void test_2() {
    char *input = "";
    cl_getline_set_str(input);

    assert_cl_getline_eof();
}

void cl_getline_test() {
    test_1();
    test_2();
}

