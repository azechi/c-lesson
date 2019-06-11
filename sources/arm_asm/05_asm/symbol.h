#ifndef SYMBOL_H
#define SYMBOL_H


#include "substring.h"


extern int mnemonic_mov;
extern int mnemonic_raw;
extern int mnemonic_ldr;
extern int mnemonic_str;


void prepare_mnemonic_symbol();

void clear_mnemonic_symbol();
void clear_label_symbol();

int find_label_symbol(const Substring *s);

int to_mnemonic_symbol(const Substring *s);
int to_label_symbol(const Substring *s);

void symbol_test();


#endif
