#ifndef ICODE_H
#define ICODE_H

#include "../symbol_table/symbol_table.h"

typedef enum {
    assign_op, add_op, sub_op, mul_op, div_op, mod_op,
    uminus_op, and_op, or_op, not_op, if_eq_op, if_noteq_op,
    if_lesseq_op, if_greatereq_op, if_less_op, if_greater_op,
    call_op, param_op, ret_op, getretval_op, funcstart_op, funcend_op,
    tablecreate_op, tablegetelem_op, tablesetelem_op,
} IOPCodeType;

typedef enum {
    var_e,
    tableitem_e,
    programfunc_e,
    libraryfunc_e,
    arithmexpr_e,
    boolexpr_e,
    assignexpr_e,
    newtable_e,
    constnum_e,
    constbool_e,
    conststring_e,
    nil_e,
} ExprType;

typedef struct Expr Expr;
typedef struct Call Call;
typedef struct Indexed Indexed;

Expr*
icode_getLvalueExpr(SymbolTableEntry* entry);

Expr*
icode_newExpr(ExprType type);

Call*
icode_newCall();

Indexed*
icode_newIndexedElem(Expr* key, Expr* value);

void
icode_setEList(Call* c, Expr* elist);

void
icode_setMethod(Call* c, unsigned char m);

void
icode_setName(Call* c, char* name);

unsigned char
icode_getMethod(Call* c);

char*
icode_getName(Call* c);

Expr*
icode_getElist(Call* c);

Expr*
icode_newConstString(char* s);

Expr*
icode_newConstNum(double i);

Expr*
icode_newConstBoolean(unsigned char bool);

unsigned char
icode_getBoolConst(Expr* e);

void
icode_setExprEntry(Expr* e, SymbolTableEntry* entry);

void
icode_setExprIndex(Expr* e, Expr* index);

void
icode_setExprType(Expr* e, ExprType type);

void
icode_setExprNext(Expr* e, Expr* next);

void
icode_setIndexedNext(Indexed* indexed, Indexed* next);

SymbolTableEntry*
icode_getExprEntry(Expr* e);

ExprType
icode_getExprType(Expr* e);

Expr*
icode_getExprIndex(Expr* e);

Expr*
icode_getExprNext(Expr* e);

Indexed*
icode_getIndexedNext(Indexed* indexed);

char*
icode_getStringConst(Expr* e);

double
icode_getNumConst(Expr* e);

Expr*
icode_reverseExprList(Expr* head);

Expr*
icode_insertFirst(Expr* elist, Expr* e);

void
icode_checkArithmetic(Expr* e, const char* context);

Expr*
icode_getIndexedKey(Indexed* indexed);

Expr*
icode_getIndexedValue(Indexed* indexed);

#endif