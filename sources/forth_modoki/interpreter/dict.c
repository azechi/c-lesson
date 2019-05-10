#include <stdlib.h>
#include <assert.h>
#include "clesson.h"

#define TABLE_SIZE 16

struct Node {
    const char *key;
    struct Element value;
    struct Node *next;
};
static struct Node *array[TABLE_SIZE];

static int hash(const char *str) {
    unsigned int val = 0;
    while(*str) {
        val += *str++;
    }
    return (int)(val % TABLE_SIZE);
}

static struct Node *new_node(const char* key, const struct Element *value) {
    struct Node *n = malloc(sizeof(struct Node));
    n->key = key;
    n->value = *value;
    n->next = NULL;
    return n;
}

static void update_or_insert_list(struct Node **head, const char *key, const struct Element *value) {
    while(*head) {
        if(streq(key, (*head)->key)){
            (*head)->value = *value;
            return;
        }

        head = &(*head)->next;
    }

    *head = new_node(key, value);
}


void dict_put(const char* key, const struct Element *el) {
    int h = hash(key);
    update_or_insert_list(&array[h], key, el);
}


int dict_get(const char* key, struct Element *out_el) {
    int h = hash(key);
    struct Node *head = array[h];

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
        struct Node *head = array[i];
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
        struct Node *node = array[i];
        while(node) {
            struct Node *tmp = node->next;
            free(node);
            node = tmp;
        }
        array[i] = NULL;
    }
}


/* unit tests */

static void assert_dict_contains(const char *key, const struct Element *expect) {
    struct Element el = {0};
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

    struct Element dummy = {0};

    dict_clear();
    dict_put("key", &dummy);
    int actual = dict_get("key", &dummy);

    assert(expect == actual);
}

static void test_dict_put_get() {
    struct Element input = {ELEMENT_LITERAL_NAME, .u.name = "abc"};
    struct Element expect = {ELEMENT_LITERAL_NAME, .u.name = "abc"};

    dict_clear();
    dict_put("key", &input);

    assert_dict_contains("key", &expect);
}

static void test_dict_append_key() {
    struct Element input_1 = {ELEMENT_LITERAL_NAME, .u.name = "abc"};
    struct Element input_2 = {ELEMENT_NUMBER, .u.number = 1};
    struct Element expect_1 = {ELEMENT_LITERAL_NAME, .u.name = "abc"};
    struct Element expect_2 = {ELEMENT_NUMBER, .u.number = 1};

    dict_clear();
    dict_put("p", &input_1);
    dict_put("o", &input_2);

    assert_dict_contains("p", &expect_1);
    assert_dict_contains("o", &expect_2);
}

static void test_dict_append_key_hash_collision() {
    struct Element input_1 = {ELEMENT_LITERAL_NAME, .u.name = "xyz"};
    struct Element input_2 = {ELEMENT_NUMBER, .u.number = 9};
    struct Element expect_1 = {ELEMENT_LITERAL_NAME, .u.name = "xyz"};
    struct Element expect_2 = {ELEMENT_NUMBER, .u.number = 9};

    dict_clear();
    dict_put("key", &input_1);
    dict_put("kye", &input_2); /* this key causes a hash collision that depends on hash algorithm */

    assert_dict_contains("key", &expect_1);
    assert_dict_contains("kye", &expect_2);
}

static void test_dict_overwrite() {
    struct Element input_1 = {ELEMENT_NUMBER, .u.number = 1};
    struct Element input_2 = {ELEMENT_LITERAL_NAME, .u.name = "abc"};
    struct Element expect = {ELEMENT_LITERAL_NAME, .u.name= "abc"};

    dict_clear();
    dict_put("key", &input_1);
    dict_put("key", &input_2);

    assert_dict_contains("key", &expect);
}

__attribute__((unused))
static void test_all() {
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
