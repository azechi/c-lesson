#include <stdlib.h>
#include <assert.h>
#include "clesson.h"

#define TABLE_SIZE 16

struct Node {
    char *key;
    struct Element value;
    struct Node *next;
};
static struct Node *array[TABLE_SIZE];

static int hash(char *str) {
    unsigned int val = 0;
    while(*str) {
        val += *str++;
    }
    return (int)(val % TABLE_SIZE);
}

static struct Node *new_node(char* key, struct Element *value) {
    struct Node *n = malloc(sizeof(struct Node));
    n->key = key;
    n->value = *value;
    n->next = NULL;
    return n;
}

static void update_or_insert_list(struct Node **head, char *key, struct Element *value) {
    while(*head) {
        if(streq(key, (*head)->key)){
            (*head)->value = *value;
            return;
        }

        head = &(*head)->next;
    }

    *head = new_node(key, value);
}


void dict_put(char* key, struct Element *el) {
    int h = hash(key);
    update_or_insert_list(&array[h], key, el);
}

int dict_get(char* key, struct Element *out_el) {
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
            switch(head->value.etype) {
                case ELEMENT_NUMBER:
                    printf("KEY: %s NUMBER: %d\n", head->key, head->value.u.number);
                    break;
                case ELEMENT_LITERAL_NAME:
                    printf("KEY: %s LITERAL_NAME: %s\n", head->key, head->value.u.name);
                    break;
                default:
                    printf("KEY: %s UNKNOWN TYPE %d\n", head->key, head->value.etype);
                    break;
            }
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
    struct Element actual = {0};
    dict_get("key", &actual);

    assert(element_equals(expect, actual));
}

static void test_dict_append_key() {
    struct Element input_1 = {ELEMENT_LITERAL_NAME, .u.name = "abc"};
    struct Element input_2 = {ELEMENT_NUMBER, .u.number = 1};
    struct Element expect_1 = {ELEMENT_LITERAL_NAME, .u.name = "abc"};
    struct Element expect_2 = {ELEMENT_NUMBER, .u.number = 1};

    dict_clear();
    dict_put("p", &input_1);
    dict_put("o", &input_2);

    struct Element actual_1 = {0};
    struct Element actual_2 = {0};
    int actual_key_exists_1 = dict_get("p", &actual_1);
    int actual_key_eixsts_2 = dict_get("o", &actual_2);

    assert(actual_key_exists_1
            && actual_key_eixsts_2
            && element_equals(expect_1, actual_1)
            && element_equals(expect_2, actual_2));

}

static void test_dict_append_key_hash_collision() {
    char *input_1 = "key";
    char *input_2 = "kye"; /* depends on hashing algorithm */
    char *expect_1 = "key";
    char *expect_2 = "kye";


    dict_clear();
    dict_put(input_1, &(struct Element){ELEMENT_NUMBER, .u.number = 1});
    dict_put(input_2, &(struct Element){ELEMENT_NUMBER, .u.number = 2});

    struct Element dummy = {0};

    int actual_1 = dict_get(expect_1, &dummy);
    int actual_2 = dict_get(expect_2, &dummy);

    assert(actual_1 && actual_2);
}

static void test_dict_overwrite() {
    struct Element input_1 = {0};
    struct Element input_2 = {0};
    struct Element expect = {0};

    dict_clear();
    dict_put("key", &input_1);
    dict_put("key", &input_2);

    struct Element actual = {0};
    dict_get("key", &actual);

    assert(element_equals(expect, actual));
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
