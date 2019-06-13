#include <assert.h>
#include <string.h>
#include <stdlib.h>

#include "substring.h"
#include "assembler_symbol_dict.h"


/* binary search tree  */
typedef struct Node_ {
    char *id;
    int value;
    struct Node_ *left;
    struct Node_ *right;
} Node;

static Node *mnemonic_root = NULL;
static Node *label_root = NULL;
int mnemonic_value = 1;
int label_value = 10001;

static Node **find(Node **root, const Substring *s);
static Node *insert(Node **root, const Substring *s);


void mnemonic_symbol_clear() {
    mnemonic_root = NULL;
    /* TODO free Node */
}

void label_symbol_clear() {
    label_root = NULL;
    /* TODO free Node */
}

int find_label_symbol(const Substring *s) {
    Node *node = *find(&label_root, s);
    if(node) {
        return node->value;
    }
    return 0;
}

int to_label_symbol(const Substring *s) {
    Node *node = insert(&label_root, s);
    if(node->value == 0) {
        node->value = label_value++;
    }
    return node->value;
}

int to_mnemonic_symbol(const Substring *s) {
    Node *node = insert(&mnemonic_root, s);
    if(node->value == 0) {
        node->value = mnemonic_value++;
    }
    return node->value;
}

static Node **find(Node **root, const Substring *s) {
    while(*root) {
        int cmp = str_cmp_subs((*root)->id, s);
        if(cmp == 0) {
            return root;
        } else if(cmp < 0) {
            root = &(*root)->left;
        } else {
            root = &(*root)->right;
        }
    }
    return root;
}

static Node *insert(Node **root, const Substring *s) {
    root = find(root, s);

    if(*root == NULL) {
        int size = s->len;
        char *id = malloc(size + 1);
        strncpy(id, s->str, size);
        id[size] = '\0';

        Node *node = malloc(sizeof(Node));
        node->id = id;
        node->value = 0;
        node->left = node->right = NULL;

        *root = node;
    }

    return *root;
}


static void verify_to_mnemonic_symbol(char *input, char *expect) {
    mnemonic_symbol_clear();
    Substring input_subs = {.str = input, .len = strlen(input)};
    Substring expect_subs = {.str = expect, .len = strlen(expect)};
    int actual = to_mnemonic_symbol(&input_subs);
    int eq = to_mnemonic_symbol(&expect_subs) == actual;

    assert(eq);
}

static void verify_to_mnemonic_symbol_not_match(char *input, char  *expect) {
    mnemonic_symbol_clear();
    Substring input_subs = {.str = input, .len = strlen(input)};
    Substring expect_subs = {.str = expect, .len = strlen(expect)};
    int actual = to_mnemonic_symbol(&input_subs);
    int eq = to_mnemonic_symbol(&expect_subs) == actual;

    assert(!eq);
}

void symbol_test() {
    verify_to_mnemonic_symbol("mov", "mov");
    verify_to_mnemonic_symbol(".raw", ".raw");
    verify_to_mnemonic_symbol_not_match("mo", "mov");
    verify_to_mnemonic_symbol_not_match("move", "mov");
}
