#ifndef ASSEMBLER_EMITTER_H
#define ASSEMBLER_EMITTER_H


typedef struct Emitter_ Emitter;

Emitter *emitter_new(char *const buf);
void emitter_free(Emitter *emitter);


void emitter_emit_word(Emitter *emitter, int word);
void emitter_patch_word(Emitter *emitter, int address ,int word);
int emitter_current_address(Emitter *emitter);

#endif
