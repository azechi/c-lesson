#ifndef ASSEMBLY_DELAY_EMIT_WORD_LIST_H
#define ASSEMBLY_DELAY_EMIT_WORD_LIST_H

typedef enum LiteralPoolItemType_ {
    POOLITEM_CONSTANT,
    POOLITEM_LABEL_REFERENCE
} LiteralPoolItemType;

typedef struct DelayEmitWord_ {
    int label;
    LiteralPoolItemType type;
    union {
        int constant;
        int target_label;
    } u;
} DelayEmitWord;

void delay_emit_word_clear();
void delay_emit_word_push(DelayEmitWord *item);
int delay_emit_word_pop(DelayEmitWord *out_item);


#endif
