#include <stdio.h>
#include <string.h>
#include "cl_getline.h"

#define BUF_SIZE 80
static char buf[BUF_SIZE];
static FILE *fp = NULL;


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
        return EOF;
    }

    return EOF;
}

