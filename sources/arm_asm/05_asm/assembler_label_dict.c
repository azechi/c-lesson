#include <stdlib.h>
#include "assembler_label_dict.h"


typedef struct LabelDictEntry_ {
    int symbol;
    int pos;
} LabelDictEntry;


static int label_dict_pos = 0;
static LabelDictEntry label_dict_array[1024];

static int label_dict_find_index(int symbol);

void label_dict_clear() {
    label_dict_pos = 0;
}

void label_dict_put(int symbol, int pos) {
    int idx = label_dict_find_index(symbol);
    if(idx == -1) {
        idx = label_dict_pos++;
    }
    label_dict_array[idx].symbol = symbol;
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


static int label_dict_find_index(int symbol) {
    int i;
    for(i = 0; i < label_dict_pos; i++) {
        if(label_dict_array[i].symbol == symbol) {
            return i;
        }
    }
    return -1;
}

