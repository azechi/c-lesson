#include "label_dictionary.h"


typedef struct LabelDictEntry_ {
    int symbol;
    int pos;
} LabelDictEntry;


static int label_dict_pos = 0;
static LabelDictEntry label_dict_array[1024];


static int label_dict_find_index(int symbol) {
    int i;
    for(i = 0; i < label_dict_pos; i++) {
        if(label_dict_array[i].symbol == symbol) {
            return i;
        }
    }
    return -1;
}


void label_dict_put(int symbol, int pos) {
    int idx = label_dict_find_index(symbol);
    if(idx == -1) {
        label_dict_array[label_dict_pos++].pos = pos;
        return;
    }
    label_dict_array[idx].pos = pos;
}


int label_dict_get(int symbol, int *out_pos) {
    int idx = label_dict_find_index(symbol);
    if(idx == -1) {
        return 0;
    }
    *out_pos = label_dict_array[idx].pos;
    return 1;
}


