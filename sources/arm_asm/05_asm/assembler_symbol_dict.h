#ifndef ASSEMBLER_SYMBOL_DICT_H
#define ASSEMBLER_SYMBOL_DICT_H


#include "substring.h"


void mnemonic_symbol_clear();
void label_symbol_clear();

int find_label_symbol(const Substring *s);

int to_mnemonic_symbol(const Substring *s);
int to_label_symbol(const Substring *s);


#endif
