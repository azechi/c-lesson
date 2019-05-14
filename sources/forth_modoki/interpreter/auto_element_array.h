#ifndef AUTO_ELEMENT_ARRAY_H
#define AUTO_ELEMENT_ARRAY_H

#include "element.h"

typedef struct AutoElementArray_ {
    int size;
    struct ElementArray_ *var_array;
} AutoElementArray;


void auto_element_array_init(AutoElementArray *out);


void auto_element_array_add_element(AutoElementArray * array, const Element *el);

void auto_element_array_trim_to_size(AutoElementArray *ae);


void auto_element_array_print();


void auto_element_array_test_all();


#endif
