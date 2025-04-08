#include "scope_space.h"

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

unsigned programVarOffset = 0;
unsigned functionLocalOffset = 0;
unsigned formalArgOffset = 0;
unsigned scopeSpaceCounter = 1;

#define RESET_LOCAL_OFFSET() (functionLocalOffset = 0)
#define RESET_FORMAL_OFFSET() (formalArgOffset = 0)

#define INCREMENT_SCOPE_COUNTER() (++scopeSpaceCounter)
#define DECREMENT_SCOPE_COUNTER() (--scopeSpaceCounter)

#define MAX_SIZE 100

typedef struct LocalOffsetStack {
    unsigned arr[MAX_SIZE];
    unsigned int length;
} LocalOffsetStack;

LocalOffsetStack* localOffsetStack = NULL;

#define PUSH_LOCAL_OFFSET() (localOffsetStack->arr[localOffsetStack->length++] = functionLocalOffset)
#define POP_AND_RESTORE_LOCAL_OFFSET() (functionLocalOffset = localOffsetStack->arr[--localOffsetStack->length])

void
scopeSpace_initialize() {
    localOffsetStack = malloc(sizeof(LocalOffsetStack));

    if (!localOffsetStack) {
        printf("Error allocating memory for local offset stack.\n");
        exit(1);
    }

    localOffsetStack->length = 0;
}

void
scopeSpace_cleanup() {
    if (localOffsetStack) {
        free(localOffsetStack);
    }
}

ScopeSpaceType
scopeSpace_currentScope() {
    if (scopeSpaceCounter == 1) {
        return PROGRAMVAR;
    }
    else if (scopeSpaceCounter % 2 == 0) {
        return FORMALARG;
    }
    else {
        return FUNCTIONLOCAL;
    }
}

unsigned
scopeSpace_currentScopeOffset() {
    switch (scopeSpace_currentScope()) {
        case PROGRAMVAR : return programVarOffset;
        case FUNCTIONLOCAL : return functionLocalOffset;
        case FORMALARG : return formalArgOffset;
        default: assert(0);
    }
}

void
scopeSpace_incrementCurrentScopeOffset() {
    switch (scopeSpace_currentScope()) {
        case PROGRAMVAR : ++programVarOffset; break;
        case FUNCTIONLOCAL : ++functionLocalOffset; break;
        case FORMALARG : ++formalArgOffset; break;
        default : assert(0);
    }
}

void
scopeSpace_enterScopeSpace() {
    INCREMENT_SCOPE_COUNTER();
}

void
scopeSpace_exitFuncBlock() {
    assert(scopeSpaceCounter > 2);

    DECREMENT_SCOPE_COUNTER();
    DECREMENT_SCOPE_COUNTER();

    if (localOffsetStack->length > 0) {
        POP_AND_RESTORE_LOCAL_OFFSET();
    }
}

void
scopeSpace_enterFuncFormalScope() {
    if (scopeSpace_currentScope() == FUNCTIONLOCAL) {
        PUSH_LOCAL_OFFSET();
        RESET_LOCAL_OFFSET();
        RESET_FORMAL_OFFSET();
    }
    INCREMENT_SCOPE_COUNTER();
}




