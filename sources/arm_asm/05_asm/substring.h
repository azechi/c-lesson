#ifndef SUBSTRING_H
#define SUBSTRING_H


typedef struct Substring_ {
    const char *str;
    int len;
} Substring;

int str_eq_subs(const char *s, const Substring *subs);
int str_cmp_subs(const char *s, const Substring *subs);

#endif
