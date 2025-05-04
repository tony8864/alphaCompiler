#include "func_stack.h"

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#define STACK_SIZE 1024

typedef struct FuncStack {
    RetList* arr[STACK_SIZE];
    int top;
} FuncStack;

typedef struct RetList {
    RetNode* head;
} RetList;

typedef struct RetNode {
    unsigned int ret;
    RetNode* next;
} RetNode;

FuncStack*
funcStack_initialize() {
    FuncStack* stack;

    stack = malloc(sizeof(FuncStack));

    if (!stack) {
        printf("Error allocating memory for func stack.\n");
        exit(1);
    }

    stack->top = 0;

    return stack;
}

void
funcStack_pushList(FuncStack* stack) {
    assert(stack->top < STACK_SIZE);

    RetList* rlist;

    rlist = malloc(sizeof(RetList));

    if (!rlist) {
        printf("Error allocating memory for return list.\n");
        exit(1);
    }

    rlist->head = NULL;

    stack->arr[stack->top++] = rlist;
}

RetList*
funcStack_top(FuncStack* stack) {
    assert(stack);
    return stack->arr[stack->top - 1];
}

RetList*
funcStack_pop(FuncStack* stack) {
    assert(stack);
    return stack->arr[--stack->top];
}

void
funcStack_appendToReturnList(RetList* rlist, unsigned int n) {
    RetNode* curr;
    RetNode* prev;
    RetNode* new;

    curr = rlist->head;
    prev = NULL;

    while (curr) {
        prev = curr;
        curr = curr->next;
    }

    new = malloc(sizeof(RetNode));
    
    if (!new) {
        printf("Error allocating memory for ret node.\n");
        exit(1);
    }

    new->ret = n;
    new->next = NULL;

    if (prev == NULL) {
        rlist->head = new;
    }
    else {
        prev->next = new;
    }
}


RetNode*
funcStack_getRetNodeHead(RetList* rlist) {
    assert(rlist);
    return rlist->head;
}

RetNode*
funcStack_getRetNodeNext(RetNode* node) {
    assert(node);
    return node->next;
}

unsigned int
funcStack_getRetNodeValue(RetNode* node) {
    assert(node);
    return node->ret;
}

void
funcStack_printRetList(RetList* rlist) {
    assert(rlist);
    RetNode* curr;
    curr = rlist->head;
    printf("Return list: ");
    while (curr) {
        printf("%u ", curr->ret);
        curr = curr->next;
    }
    printf("\n");
}

void
funcStack_freeRetList(RetList* rlist) {
    RetNode* curr = rlist->head;
    while (curr) {
        RetNode* next = curr->next;
        free(curr);
        curr = next;
    }
    free(rlist);
}