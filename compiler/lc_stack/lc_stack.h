#ifndef LC_STACK
#define LC_STACK

typedef struct LcStack LcStack;

void
lcStack_initialize();

void
lcStack_incrementLoopCounter();

void
lcStack_decrementLoopCounter();

void
lcStack_pushLoopCounter();

void
lcStack_popLoopCouter();

unsigned
lcStack_isInsideLoop();

void
lcStack_cleanup();

#endif