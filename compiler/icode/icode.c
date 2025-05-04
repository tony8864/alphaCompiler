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

typedef struct Indexed {
    Expr* key;
    Expr* value;
    Indexed* next;
} Indexed;

typedef struct ForPrefix {
    unsigned int test;
    unsigned int enter;
} ForPrefix;

typedef struct Statement {
    int breakList;
    int contList;
} Statement;

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

Indexed*
icode_newIndexedElem(Expr* key, Expr* value) {
    Indexed* indexed;

    indexed = malloc(sizeof(Indexed));

    if (!indexed) {
        printf("Error creating new indexed element.\n");
        exit(1);
    }

    indexed->key = key;
    indexed->value = value;
    indexed->next = NULL;

    return indexed;
}

Statement*
icode_newStatement() {
    Statement* stmt;

    stmt = malloc(sizeof(Statement));

    if (!stmt) {
        printf("Error allocating memory for statement.\n");
        exit(1);
    }

    stmt->breakList = 0;
    stmt->contList = 0;

    return stmt;
}

void
icode_setContList(Statement* stmt, int i) {
    assert(stmt);
    stmt->contList = i;
}

void
icode_setBreakList(Statement* stmt, int i) {
    assert(stmt);
    stmt->breakList = i;
}

int
icode_getBreakList(Statement* stmt) {
    assert(stmt);
    return stmt->breakList;
}

int
icode_getContList(Statement* stmt) {
    assert(stmt);
    return stmt->contList;
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
    while(elist) {
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

Expr*
icode_newConstBoolean(unsigned char bool) {
    Expr* e;

    e = icode_newExpr(constbool_e);
    e->constValue.boolean = (bool != 0) ? 1 : 0;

    return e;
}

unsigned char
icode_getBoolConst(Expr* e) {
    assert(e);
    return e->constValue.boolean;
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

void
icode_setIndexedNext(Indexed* indexed, Indexed* next) {
    assert(indexed);
    indexed->next = next;
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

Indexed*
icode_getIndexedNext(Indexed* indexed) {
    assert(indexed);
    return indexed->next;
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

void
icode_checkArithmetic(Expr* e, const char* context, unsigned int line) {
    if (    e->type == constbool_e      ||
            e->type == conststring_e    ||
            e->type == nil_e            ||
            e->type == newtable_e       ||
            e->type == programfunc_e    ||
            e->type == libraryfunc_e    ||
            e->type == boolexpr_e ) {
                printf("Illegal expr at line [%u] used in %s\n", line, context);
                exit(1);
            }
}

Expr*
icode_getIndexedKey(Indexed* indexed) {
    assert(indexed);
    return indexed->key;
}

Expr*
icode_getIndexedValue(Indexed* indexed) {
    assert(indexed);
    return indexed->value;
}

ForPrefix*
icode_newForPrefix(unsigned int test, unsigned int enter) {
    ForPrefix* forPrefix;

    forPrefix = malloc(sizeof(ForPrefix));
    
    if (!forPrefix) {
        printf("Error allocating memory for forprefix.\n");
        exit(1);
    }

    forPrefix->test = test;
    forPrefix->enter = enter;

    return forPrefix;
}

unsigned int
icode_getForPrefixTest(ForPrefix* f) {
    assert(f);
    return f->test;
}

unsigned int
icode_getForPrefixEnter(ForPrefix* f) {
    assert(f);
    return f->enter;
}