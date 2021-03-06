#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "auto_element_array.h"


void auto_element_array_init(AutoElementArray *out) {
    out->var_array = new_element_array(out->size);
}

void auto_element_array_add_element(AutoElementArray *aea, const Element *el) {

    if(aea->size == aea->var_array->len) {
        aea->size = (aea->size + 1) * 2;
        int ea_size = sizeof(ElementArray) + (sizeof(Element) * aea->size);
        ElementArray *new_ea = realloc(aea->var_array, ea_size);
        aea->var_array = new_ea;
    }

    aea->var_array->elements[aea->var_array->len++] = *el;
}

void auto_element_array_trim_to_size(AutoElementArray *ae) {
    int len = ae->var_array->len;
    int size = sizeof(ElementArray) + sizeof(Element) * len;
    ElementArray *ar = realloc(ae->var_array, size);
    ae->var_array = ar;
}


void auto_element_array_print(const AutoElementArray *aea) {
    printf("size %d\n", aea->size);
    element_array_print(aea->var_array);
}


/* Unit test */
static void verify_init(int input);
static void verify_size_increase(int input_count, int expect_size);
static void verify_add_element(int input_len, Element input_elements[]);
#define VERIFY_ADD_ELEMENT(a) verify_add_element(sizeof(a) / sizeof(Element), a)


static void test_init() {
    verify_init(0);
    verify_init(1);
}

static void test_size_increase() {
    /* below is expecting values that depend on an increase algorithm */
    verify_size_increase(0, 0);
    verify_size_increase(1, 2);
    verify_size_increase(3, 6);
    verify_size_increase(7, 14);
    verify_size_increase(15, 30);
}

static void test_add_element() {
    Element input[] = {
        {ELEMENT_NUMBER, .u.number = 1},
        {ELEMENT_NUMBER, .u.number = 2},
        {ELEMENT_NUMBER, .u.number = 3}
    };

    VERIFY_ADD_ELEMENT(input);
}


void auto_element_array_test_all() {
    test_init();
    test_size_increase();
    test_add_element();
}


static void verify_init(int input) {
    AutoElementArray aea = {input};
    auto_element_array_init(&aea);

    assert(input == aea.size);
    assert(0 == aea.var_array->len);
}

static void verify_size_increase(int input_count, int expect_size) {
    AutoElementArray aea = {0};
    auto_element_array_init(&aea);

    int i = input_count;
    while(i--) {
        Element dummy = {0};
        auto_element_array_add_element(&aea, &dummy);
    }

    assert(input_count == aea.var_array->len);
    assert(expect_size == aea.size);
}

static void verify_add_element(int input_len, Element input_elements[]) {
    ElementArray *expect = new_element_array(input_len);
    memcpy(expect->elements, input_elements, sizeof(Element) * input_len);
    expect->len = input_len;

    AutoElementArray aea = {0};
    auto_element_array_init(&aea);

    int i;
    for(i = 0; i < input_len; i++) {
        auto_element_array_add_element(&aea, &input_elements[i]);
    }

    int eq = element_array_equals(expect, aea.var_array);
    assert(eq);
}

