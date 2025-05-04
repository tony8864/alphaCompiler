#ifndef SCOPE_STACK_H
#define SCOPE_STACK_H

typedef struct ScopeStack ScopeStack;
typedef enum { SCOPE_BLOCK, SCOPE_FUNCTION, SCOPE_GLOBAL, SCOPE_NONE } ScopeType;

void
scopeStack_initialize();

void
scopeStack_push(ScopeType type);

void
scopeStack_pop();

int
scopeStack_isAccessible(unsigned int startScope, unsigned int endScope);

void
scopeStack_print();

void
scopeStack_cleanup();

#endif