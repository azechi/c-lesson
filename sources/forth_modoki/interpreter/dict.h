#ifndef DICT_H
#define DICT_H

#include "element.h"


/*
   return 1 if key exists
   return 0 if key not exists
   */
int dict_get(const char* key, Element *out_el);

void dict_put(const char* key, const Element *el);

void dict_put_c_func(const char *name, void(*c_func)());


void dict_clear();

void dict_print_all();

void dict_test_all();


/*
   return 1 if key exists
   return 0 if key not exists
   */
int compile_dict_get(const char* key, Element *out_el);

void compile_dict_put(const char* key, const Element *el);

void compile_dict_put_compile_func(const char *name, CompileFunc compile_func);


void compile_dict_clear();

#endif
