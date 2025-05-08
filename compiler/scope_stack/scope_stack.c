#include "scope_stack.h"

#include <stdio.h>
#include <stdlib.h>

#define MAX_SIZE 100

typedef struct ScopeStack {
    ScopeType arr[MAX_SIZE];
    unsigned int length;
} ScopeStack;

ScopeStack* scopeStack = NULL;

/* ======================================== IMPLEMENTATION ======================================== */
void
scopeStack_initialize() {
    scopeStack = malloc(sizeof(ScopeStack));

    if (!scopeStack) {
        printf("Error allocating memory for scope stack.\n");
        exit(1);
    }

    for (int i = 0; i < MAX_SIZE; i++) {
        scopeStack->arr[i] = SCOPE_NONE;
    }
    scopeStack->length = 0;
}

void
scopeStack_push(ScopeType type) {
    if (scopeStack && scopeStack->length < MAX_SIZE) {
        scopeStack->arr[scopeStack->length++] = type;
    }
}

void
scopeStack_pop() {
    if (scopeStack && scopeStack->length > 0) {
        scopeStack->length--;
    }
}

int
scopeStack_isAccessible(unsigned int startScope, unsigned int endScope) {
    int isAccessible = 1;
    for (int i = endScope; i >= startScope; i--) {
        if (i < scopeStack->length &&  scopeStack->arr[i] == SCOPE_FUNCTION) {
            printf("wtf: %d\n", i);
            isAccessible = 0;
            break;
        }
    }
    return isAccessible;
}

void
scopeStack_print() {
    if (scopeStack) {
        printf("scope stack (%d): ", scopeStack->length);
        for (int i = 0; i < scopeStack->length; i++) {
            if (scopeStack->arr[i] == SCOPE_BLOCK) {
                printf("B ");
            }
            else if (scopeStack->arr[i] == SCOPE_FUNCTION) {
                printf("F ");
            }
            else if (scopeStack->arr[i] == SCOPE_GLOBAL){
                printf("G ");
            }
        }
        printf("\n");
    }
}

void
scopeStack_cleanup() {
    if (scopeStack) {
        free(scopeStack);
    }
}