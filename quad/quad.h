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