#ifndef FUNC_STACK
#define FUNC_STACK

typedef struct FuncStack FuncStack;
typedef struct RetList RetList;
typedef struct RetNode RetNode;

FuncStack*
funcStack_initialize();

void
funcStack_pushList(FuncStack* stack);

RetList*
funcStack_top(FuncStack* stack);

RetList*
funcStack_pop(FuncStack* stack);

void
funcStack_appendToReturnList(RetList* rlist, unsigned int n);

void
funcStack_printRetList(RetList* rlist);

RetNode*
funcStack_getRetNodeHead(RetList* rlist);

RetNode*
funcStack_getRetNodeNext(RetNode* node);

unsigned int
funcStack_getRetNodeValue(RetNode* node);

void
funcStack_freeRetList(RetList* rlist);

#endif