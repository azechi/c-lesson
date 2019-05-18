#ifndef COMPILER_H
#define COMPILER_H


#include "element.h"


void register_primitive();


Element compile_exec_array(int ch, int *out_ch);


#endif
