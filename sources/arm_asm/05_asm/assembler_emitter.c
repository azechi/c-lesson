#include <stdlib.h>
#include <string.h>
#include "substring.h"
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

void emitter_emit_string(Emitter *emitter, const char *s) {
    int i = strlen(s) + 1;
    for(; i > 0; i--) {
        emitter->buf[emitter->pos++] = *(s++);
    }

    i = emitter->pos % 4;
    i = (i == 0)? i: 4 - i;
    for(; i > 0; i--) {
        emitter->buf[emitter->pos++] = '\0';
    }
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

