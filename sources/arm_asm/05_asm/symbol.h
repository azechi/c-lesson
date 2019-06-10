#ifndef SYMBOL_H
#define SYMBOL_H

#include "substring.h"

extern int mnemonic_mov;
extern int mnemonic_raw;
extern int mnemonic_ldr;
extern int mnemonic_str;


void init_symbol();

int to_mnemonic_symbol(const Substring *s);

void symbol_test();


#endif
