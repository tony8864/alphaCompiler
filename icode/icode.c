#include "../symbol_table/symbol_table.h"
#include "icode.h"

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

typedef union ConstValue {
    double num;
    char* str;
    unsigned char boolean;
} ConstValue;

typedef struct Expr {
    ExprType type;
    SymbolTableEntry* entry;
    Expr* index;
    ConstValue constValue;
    Expr* next;
} Expr;

/* ------------------------------ Implementation ------------------------------ */
Expr*
icode_getLvalueExpr(SymbolTableEntry* entry) {
    assert(entry);
    
    Expr* e = malloc(sizeof(Expr));

    memset(e, 0, sizeof(Expr));

    e->next = NULL;
    e->entry = entry;

    switch(symtab_getEntryType(entry)) {
        case GLOBAL:
        case LOCAL_T:
        case FORMAL:
            e->type = var_e;
            break;
        case LIBFUNC:
            e->type = libraryfunc_e;
            break;
        case USERFUNC:
            e->type = programfunc_e;
            break;
        default:
            assert(0);
    }
    
    return e;
}

Expr*
icode_newExpr(ExprType type) {
    Expr* e;

    e = malloc(sizeof(Expr));

    if (!e) {
        printf("Error allocating memory for new expr.\n");
        exit(1);
    }

    memset(e, 0, sizeof(Expr));
    e->type = type;
    return e;
}

Expr*
icode_newConstString(char* s) {
    Expr* e;

    e = icode_newExpr(conststring_e);
    e->constValue.str = strdup(s);

    return e;
}

Expr*
icode_newConstNum(double i) {
    Expr* e;

    e = icode_newExpr(constnum_e);
    e->constValue.num = i;

    return e;
}

void
icode_setExprEntry(Expr* e, SymbolTableEntry* entry) {
    assert(e && entry);
    e->entry = entry;
}

void
icode_setExprIndex(Expr* e, Expr* index) {
    assert(e && index);
    e->index = index;
}

SymbolTableEntry*
icode_getExprEntry(Expr* e) {
    assert(e);
    return e->entry;
}

ExprType
icode_getExprType(Expr* e) {
    assert(e);
    return e->type;
}

Expr*
icode_getExprIndex(Expr* e) {
    assert(e);
    return e->index;
}

char*
icode_getStringConst(Expr* e) {
    assert(e);
    return e->constValue.str;
}

double
icode_getNumConst(Expr* e) {
    assert(e);
    return e->constValue.num;
}