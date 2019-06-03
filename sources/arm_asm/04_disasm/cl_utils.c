#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#include "cl_utils.h"

static char buf[100*1024];

static int to_buffer = 0;
static int pos = 0;

void cl_clear_output() {
    pos = 0;
    buf[0] = '\0';
}


char *cl_get_result(int num) {
    int null_count=0;
    int i=0;
    while (null_count != num) {
        if (buf[i] == '\0') {
            null_count++;
        }
        i++;
    }
    return &buf[i];
}

void cl_enable_buffer_mode() {
    to_buffer = 1;
}

void cl_disable_buffer_mode() {
    to_buffer = 0;
}


static void printf_va(const char *fmt, va_list arg_ptr);

void cl_printfn(const char *fmt, ...) {
    va_list arg_ptr;
    va_start(arg_ptr, fmt);

    int len = strlen(fmt) + 1 + strlen("\n");
    char buff[len];
    sprintf(buff, "%s\n", fmt);
    printf_va(buff, arg_ptr);

    va_end(arg_ptr);
}

void cl_printf(char *fmt, ...) {
    va_list arg_ptr;
    va_start(arg_ptr, fmt);

    printf_va(fmt, arg_ptr);

    va_end(arg_ptr);
}


static void printf_va(const char *fmt, va_list arg_ptr) {
    if(to_buffer) {
        pos += vsprintf(&buf[pos], fmt, arg_ptr);
        pos++;
    } else {
        vprintf(fmt, arg_ptr);
    }
}

