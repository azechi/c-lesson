#include <stdio.h>
#include <string.h>

#ifndef CLESSON_H
#define CLESSON_H


#define assert_fail(msg) assert(0&&(msg))


int streq(const char *s1, const char *s2);

void print_indent(int i);



#endif
