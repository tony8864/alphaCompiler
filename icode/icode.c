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

typedef struct Call {
    Expr* elist;
    unsigned char method;
    char* name;
} Call;

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

Call*
icode_newCall() {
    Call* c;

    c = malloc(sizeof(Call));

    if (!c) {
        printf("Error allocating memory for new call.\n");
        exit(1);
    }

    memset(c, 0, sizeof(Call));
    return c;
}

void
icode_setEList(Call* c, Expr* elist) {
    assert(c);
    c->elist = elist;
}

void
icode_setMethod(Call* c, unsigned char m) {
    assert(c);
    c->method = m;
}

void
icode_setName(Call* c, char* name) {
    assert(c);
    c->name = name;
}

unsigned char
icode_getMethod(Call* c) {
    assert(c);
    return c->method;
}

char*
icode_getName(Call* c) {
    assert(c);
    return c->name;
}

Expr*
icode_getElist(Call* c) {
    assert(c);
    Expr* elist = c->elist;
    printf("here\n");
    while(elist) {
        printf("---------------name: %s\n", symtab_getEntryName(elist->entry));
        elist = elist->next;
    }
    return c->elist;
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

void
icode_setExprType(Expr* e, ExprType type) {
    assert(e);
    e->type = type;
}

void
icode_setExprNext(Expr* e, Expr* next) {
    assert(e);
    e->next = next;
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

Expr*
icode_getExprNext(Expr* e) {
    assert(e);
    return e->next;
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

Expr*
icode_reverseExprList(Expr* head) {
    Expr* prev = NULL;
    Expr* current = head;
    Expr* next = NULL;

    while (current != NULL) {
        next = current->next;
        current->next = prev;
        prev = current;
        current = next;
    }

    return prev;
}

Expr*
icode_insertFirst(Expr* elist, Expr* e) {
    e->next = elist;
    return e;
}