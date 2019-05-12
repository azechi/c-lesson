#include <stdlib.h>
#include <assert.h>
#include "util.h"
#include "element.h"

#define TABLE_SIZE 16

typedef struct Node_ {
    const char *key;
    Element value;
    struct Node_ *next;
} Node;
static Node *array[TABLE_SIZE];

static int hash(const char *str) {
    unsigned int val = 0;
    while(*str) {
        val += *str++;
    }
    return (int)(val % TABLE_SIZE);
}

static Node *new_node(const char* key, const Element *value) {
    Node *n = malloc(sizeof(Node));
    n->key = key;
    n->value = *value;
    n->next = NULL;
    return n;
}

static void update_or_insert_list(Node **head, const char *key, const Element *value) {
    while(*head) {
        if(streq(key, (*head)->key)){
            (*head)->value = *value;
            return;
        }

        head = &(*head)->next;
    }

    *head = new_node(key, value);
}


void dict_put(const char* key, const Element *el) {
    int h = hash(key);
    update_or_insert_list(&array[h], key, el);
}

void dict_put_c_func(const char *name, void(*c_func)()) {
    Element el = {ELEMENT_C_FUNC, .u.cfunc = c_func};
    dict_put(name, &el);
}

int dict_get(const char* key, Element *out_el) {
    int h = hash(key);
    Node *head = array[h];

    while(head) {
        if(streq(key,  head->key)) {
            *out_el = head->value;
            return 1;
        }
        head = head->next;
    }

    return 0;
}

void dict_print_all() {
    int i = TABLE_SIZE;
    while(i--) {
        Node *head = array[i];
        while(head) {
            printf("KEY: %s", head->key);
            element_print(&head->value);
            head = head->next;
        }
    }
}

void dict_clear() {
    int i = TABLE_SIZE;
    while(i--){
        Node *node = array[i];
        while(node) {
            Node *tmp = node->next;
            free(node);
            node = tmp;
        }
        array[i] = NULL;
    }
}


/* unit tests */

static void assert_dict_contains(const char *key, const Element *expect) {
    Element el = {0};
    int exists = dict_get(key, &el);
    int actual = element_equals(expect, &el);

    assert(exists && actual);
}


static void test_dict_key_not_exists() {
    int expect = 0;

    dict_clear();
    int actual = dict_get("key", NULL);

    assert(expect == actual);
}

static void test_dict_key_exists() {
    int expect = 1;

    Element dummy = {0};

    dict_clear();
    dict_put("key", &dummy);
    int actual = dict_get("key", &dummy);

    assert(expect == actual);
}

static void test_dict_put_get() {
    Element input = {ELEMENT_LITERAL_NAME, .u.name = "abc"};
    Element expect = {ELEMENT_LITERAL_NAME, .u.name = "abc"};

    dict_clear();
    dict_put("key", &input);

    assert_dict_contains("key", &expect);
}

static void test_dict_append_key() {
    Element input_1 = {ELEMENT_LITERAL_NAME, .u.name = "abc"};
    Element input_2 = {ELEMENT_NUMBER, .u.number = 1};
    Element expect_1 = {ELEMENT_LITERAL_NAME, .u.name = "abc"};
    Element expect_2 = {ELEMENT_NUMBER, .u.number = 1};

    dict_clear();
    dict_put("p", &input_1);
    dict_put("o", &input_2);

    assert_dict_contains("p", &expect_1);
    assert_dict_contains("o", &expect_2);
}

static void test_dict_append_key_hash_collision() {
    Element input_1 = {ELEMENT_LITERAL_NAME, .u.name = "xyz"};
    Element input_2 = {ELEMENT_NUMBER, .u.number = 9};
    Element expect_1 = {ELEMENT_LITERAL_NAME, .u.name = "xyz"};
    Element expect_2 = {ELEMENT_NUMBER, .u.number = 9};

    dict_clear();
    dict_put("key", &input_1);
    dict_put("kye", &input_2); /* this key causes a hash collision that depends on hash algorithm */

    assert_dict_contains("key", &expect_1);
    assert_dict_contains("kye", &expect_2);
}

static void test_dict_overwrite() {
    Element input_1 = {ELEMENT_NUMBER, .u.number = 1};
    Element input_2 = {ELEMENT_LITERAL_NAME, .u.name = "abc"};
    Element expect = {ELEMENT_LITERAL_NAME, .u.name= "abc"};

    dict_clear();
    dict_put("key", &input_1);
    dict_put("key", &input_2);

    assert_dict_contains("key", &expect);
}

void dict_test_all() {
    test_dict_key_not_exists();
    test_dict_key_exists();
    test_dict_append_key_hash_collision();
    test_dict_put_get();
    test_dict_append_key();
    test_dict_overwrite();
}

#if 0
int main() {

    test_all();

    return 0;
}
#endif
