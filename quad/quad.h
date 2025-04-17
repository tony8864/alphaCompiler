#ifndef QUAD_H
#define QUAD_H

#include "../icode/icode.h"

typedef struct Quad Quad;

void
quad_emit(IOPCodeType op, Expr* arg1, Expr* arg2, Expr* result, unsigned label, unsigned line);

void
quad_printQuads();


#endif