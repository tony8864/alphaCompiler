#include "lc_stack.h"

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#define SIZE 100

typedef struct LcStack {
    unsigned arr[SIZE];
    unsigned int length;
} LcStack;


LcStack* stack = NULL;

void
lcStack_initialize() {
    stack = malloc(sizeof(LcStack));

    if (!stack) {
        printf("Error allocating memory for loop counter stack.\n");
        exit(1);
    }

    for (int i = 0; i < SIZE; i++) {
        stack->arr[i] = 0;
    }

    stack->length = 0;
}

void
lcStack_incrementLoopCounter() {
    assert(stack->length > 0);
    stack->arr[stack->length - 1]++;
}

void
lcStack_decrementLoopCounter() {
    assert(stack->length > 0);
    stack->arr[stack->length - 1]--;
}

void
lcStack_pushLoopCounter() {
    assert(stack->length < SIZE);
    stack->arr[stack->length++] = 0;
}

void
lcStack_popLoopCouter() {
    assert(stack->length > 0);
    stack->length--;
}

unsigned
lcStack_isInsideLoop() {
    assert(stack->length > 0);
    return stack->arr[stack->length - 1];
}

void
lcStack_cleanup() {
    assert(stack);
    free(stack);
}