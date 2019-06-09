#ifndef ASSEMBLER_H
#define ASSEMBLER_H

typedef struct Emitter_ {
    char *buf;
    int pos;
} Emitter;

int assemble(Emitter *emitter);
void assembler_test();

#endif
