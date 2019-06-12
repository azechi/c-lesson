#include <string.h>
#include "substring.h"


int str_eq_subs(const char *s, const Substring *subs) {
    int len = strlen(s);
    return len == subs->len && strncmp(s, subs->str, len) == 0;
}

int str_cmp_subs(const char *s, const Substring *subs) {
    int cmp = strncmp(s, subs->str, subs->len);
    if(cmp == 0) {
        return strlen(s) - subs->len;
    }
    return cmp;
}

