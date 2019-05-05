#include <assert.h>
#include "clesson.h"


static int dict_pos = 0;
struct KeyValue {
    char *key;
    struct Element value;
};
static struct KeyValue dict_array[1024];

static int find_index(char *key) {
    int i;
    for(i = 0; i < dict_pos; i++) {
        if(streq(key, dict_array[i].key)) {
            return i;
        }
    }
    return -1;
}

void dict_put(char* key, struct Element *el) {
    int i = find_index(key);

    if(i != -1) {
        dict_array[i].value = *el;
        return;
    }

    dict_array[dict_pos++] = (struct KeyValue){key, *el};
}

int dict_get(char* key, struct Element *out_el) {
    int i = find_index(key);

    if (i != -1) {
        *out_el = dict_array[i].value;
        return 1;
    }

    return 0;
}

void dict_print_ll() {
    assert(0);
}

void dict_clear() {
    dict_pos = 0;
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
    dict_put("key1", &input_1);
    dict_put("key2", &input_2);

    struct Element actual_1 = {0};
    struct Element actual_2 = {0};
    int actual_key_exists_1 = dict_get("key1", &actual_1);
    int actual_key_eixsts_2 = dict_get("key2", &actual_2);

    assert(actual_key_exists_1
            && actual_key_eixsts_2
            && element_equals(expect_1, actual_1)
            && element_equals(expect_2, actual_2));

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

static void test_all() {
    test_dict_key_not_exists();
    test_dict_key_exists();
    test_dict_put_get();
    test_dict_append_key();
    test_dict_overwrite();
}


int main() {

    test_all();

    return 0;
}
