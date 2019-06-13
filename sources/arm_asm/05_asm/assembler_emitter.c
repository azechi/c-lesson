#include <stdlib.h>
#include "assembler_emitter.h"

struct Emitter_ {
    char *buf;
    int pos;
};

Emitter *emitter_new(char *const buf) {
    Emitter *e = malloc(sizeof(Emitter));
    e->buf = buf;
    e->pos = 0;
    return e;
}

void emitter_free(Emitter *emitter) {
    free(emitter);
}


void emitter_emit_word(Emitter *emitter, int word) {
    emitter->buf[emitter->pos++] = word & 0xFF;
    emitter->buf[emitter->pos++] = word >> 8 & 0xFF;
    emitter->buf[emitter->pos++] = word >> 16 & 0xFF;
    emitter->buf[emitter->pos++] = word >> 24 & 0xFF;
}

void emitter_patch_word(Emitter *emitter, int address ,int word) {
    emitter->buf[address++] |= word & 0xFF;
    emitter->buf[address++] |= word >> 8 & 0xFF;
    emitter->buf[address++] |= word >> 16 & 0xFF;
    emitter->buf[address++] |= word >> 24 & 0xFF;
}

int emitter_current_address(Emitter *emitter) {
    return emitter->pos;
}

