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