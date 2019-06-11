#include <assert.h>
#include <string.h>
#include <stdlib.h>

#include "substring.h"
#include "symbol.h"

int mnemonic_mov;
int mnemonic_raw;
int mnemonic_ldr;
int mnemonic_str;

static int string_to_mnemonic_symbol(const char *s);

void prepare_mnemonic_symbol() {
    mnemonic_mov = string_to_mnemonic_symbol("mov");
    mnemonic_raw = string_to_mnemonic_symbol(".raw");
    mnemonic_ldr = string_to_mnemonic_symbol("ldr");
    mnemonic_str = string_to_mnemonic_symbol("str");
}

static int string_to_mnemonic_symbol(const char *s) {
    Substring subs = {.str = s, .len = strlen(s)};
    return to_mnemonic_symbol(&subs);
}

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
int label_value = 0x00010001;

static Node **find(Node **root, const Substring *s);
static Node *insert(Node **root, const Substring *s);


void clear_mnemonic_symbol() {
    mnemonic_root = NULL;
    /* TODO free Node */
}

void clear_label_symbol() {
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
        char *id = malloc(sizeof(s->len) + 1);
        strncpy(id, s->str, s->len + 1);

        Node *node = malloc(sizeof(Node));
        node->id = id;
        node->value = 0;
        node->left = node->right = NULL;

        *root = node;
    }

    return *root;
}


static void verify_to_mnemonic_symbol(char *input, int expect) {
    Substring subs = {.str = input, .len = strlen(input)};
    int actual = to_mnemonic_symbol(&subs);
    assert(expect == actual);
}

static void verify_to_mnemonic_symbol_not_match(char *input, int expect) {
    Substring subs = {.str = input, .len = strlen(input)};
    int actual = to_mnemonic_symbol(&subs);
    assert(expect != actual);
}

void symbol_test() {
    prepare_mnemonic_symbol();
    verify_to_mnemonic_symbol("mov", mnemonic_mov);
    verify_to_mnemonic_symbol("mov", mnemonic_mov);
    verify_to_mnemonic_symbol(".raw", mnemonic_raw);
    verify_to_mnemonic_symbol_not_match("mo", mnemonic_mov);
    verify_to_mnemonic_symbol_not_match("move", mnemonic_mov);
}
