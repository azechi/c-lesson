#ifndef LABEL_DICTIONARY_H
#define LABEL_DICTIONARY_H


void label_dict_put(int symbol, int pos);
int label_dict_get(int symbol, int *out_pos);

#endif
