#ifndef QUAD_H
#define QUAD_H

#include "../icode/icode.h"
#include "../symbol_table/symbol_table.h"

typedef struct Quad Quad;

void
quad_emit(IOPCodeType op, Expr* arg1, Expr* arg2, Expr* result, unsigned label, unsigned line);

void
quad_emitIfTableItem(Expr* e);

void
quad_writeQuadsToFile(char* filename);

unsigned int
quad_nextQuadLabel();

unsigned int
quad_totalQuads();

IOPCodeType
quad_getOpcode(unsigned int i);

int
quad_getIndex(Quad* q);

Quad*
quad_getAt(unsigned int i);

Expr*
quad_getArg1(Quad* q);

Expr*
quad_getArg2(Quad* q);

Expr*
quad_getResult(Quad* q);

unsigned
quad_getLine(Quad* q);

unsigned
quad_getLabel(Quad* q);

unsigned
quad_getTargetAddress(Quad* q);

void
quad_setTargetAddress(Quad* q, unsigned taddress);

void
quad_patchLabel(unsigned quadNo, unsigned label);

int
quad_newList(int i);

int
quad_mergeList(int l1, int l2);

void
quad_patchList(int list, int label);

void
quad_printQuads();

void
quad_printList(int head);

#endif