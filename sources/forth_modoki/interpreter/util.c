#include <string.h>
#include <stdio.h>
#include "util.h"

int streq(const char *s1, const char *s2) {
    return (strcmp(s1, s2) == 0);
}


void print_indent(int i) {
    char str[5]; /* "%99c"" */
    sprintf(str, "%%%dc", i % 100);
    printf(str, ' ');
}
