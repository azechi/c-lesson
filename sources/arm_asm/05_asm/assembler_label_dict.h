#ifndef ASSEMBLER_LABEL_DICT_H
#define ASSEMBLER_LABEL_DICT_H

void label_dict_clear();
void label_dict_put(int symbol, int pos);
int label_dict_get(int symbol, int *out_pos);


#endif
