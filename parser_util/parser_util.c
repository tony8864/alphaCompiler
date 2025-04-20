#include "../scope_space/scope_space.h"
#include "../scope_stack/scope_stack.h"
#include "../icode/icode.h"
#include "../quad/quad.h"
#include "parser_util.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

static const char* libraryFunctions[] = {
    "print",
    "input",
    "objectmemberkeys",
    "objecttotalmembers",
    "objectcopy",
    "totalarguments",
    "argument",
    "typeof",
    "strtonum",
    "sqrt",
    "cos",
    "sin"
};

static const int NUM_LIBRARY_FUNCTIONS = sizeof(libraryFunctions) / sizeof(libraryFunctions[0]);

static unsigned int scope = 0;
static unsigned int tempCounter = 0;

#define SCOPE_ENTER()   (scope++)
#define SCOPE_EXIT()    (scope--)

SymbolTable* table = NULL;

/* ======================================== STATIC DECLARATIONS ======================================== */
static int
isSymbolLibraryFunction(const char* name);

static void
insertLibraryFunctions();

SymbolTableEntry*
handleFunctionDefinition(const char* name, unsigned int line);

void
reportInaccessibleVariable(unsigned int line1, unsigned int line2, const char* name);

char*
newTempName();

void
resetTemp();

SymbolTableEntry*
newTemp(unsigned int line);

Expr*
emit_iftableitem(Expr* e, unsigned int line);

Expr*
member_item(Expr* lv, char* identifier, unsigned int line);

Expr*
make_call(Expr* lv, Expr* reversed_elist, unsigned int line);

Expr*
assignToTableItem(Expr* lv, Expr* e, unsigned int line);

Expr*
assignToLvalue(Expr* lv, Expr* e, unsigned int line);

/* ======================================== IMPLEMENTATION ======================================== */
void
parserUtil_initialize() {
    table = symtab_initialize();
  
    insertLibraryFunctions();
    scopeStack_initialize();
    scopeSpace_initialize();
}

void
parserUtil_cleanup() {
    
    symtab_printScopeTable(table);
    scopeStack_print();
    quad_printQuads();

    quad_writeQuadsToFile("quads.txt");

    scopeStack_cleanup();
    scopeSpace_cleanup();
}

void
parserUtil_handleBlockEntrance() {
    SCOPE_ENTER();
    scopeStack_push(SCOPE_BLOCK);
}

void
parserUtil_handleBlockExit() {
    symtab_hide(table, scope);
    SCOPE_EXIT();
    scopeStack_pop();
}

Expr*
parserUtil_handleIdentifier(const char* name, unsigned int line) {
    SymbolTableEntry* entry;

    for (int currentScope = scope; currentScope >= 1; currentScope--) {
        entry = symtab_lookupInScopeTable(table, name, currentScope);

        if (entry && symtab_isEntryActive(entry)) {
            if (currentScope == scope) {
                return icode_getLvalueExpr(entry);
            }
            else if (scopeStack_isAccessible(currentScope, scope)) {
                return icode_getLvalueExpr(entry);
            }
            else {
                reportInaccessibleVariable(line, symtab_getEntryLine(entry), name);
            }
        }
    }

    entry = symtab_lookupInScopeTable(table, name, 0);

    if (entry) {
        return icode_getLvalueExpr(entry);
    }

    ScopeType type = (scope == 0) ? GLOBAL : LOCAL_T;
    return icode_getLvalueExpr(symtab_insertVariable(table, name, line, scope, type));
}

Expr*
parserUtil_handleLocalIdentifier(const char* name, unsigned int line) {
    SymbolTableEntry* entry;

    entry = symtab_lookupInScopeTable(table, name, scope);

    if (!entry || (entry && !symtab_isEntryActive(entry))) {
        if (isSymbolLibraryFunction(name)) {
            printf("Error at line %d: Shadowing library function.\n", line);
            exit(1);
        }
        
        SymbolType type = (scope == 0) ? GLOBAL : LOCAL_T;
        entry = symtab_insertVariable(table, name, line, scope, type);
    }

    return icode_getLvalueExpr(entry);
}

Expr*
parserUtil_habdleGlobalLookup(const char* name, unsigned int line) {
    SymbolTableEntry* entry;

    entry = symtab_lookupInScopeTable(table, name, 0);

    if (!entry) {
        printf("Error at line %d: Global \"%s\" not found.\n", line, name);
        exit(1);
    }

    return icode_getLvalueExpr(entry);
}

SymbolTableEntry*
parserUtil_handleFormalArgument(const char* name, unsigned int line) {
    SymbolTableEntry* entry;

    entry = symtab_lookupInScopeTable(table, name, scope);

    if (entry && symtab_isEntryActive(entry)) {
        printf("Error at line %d: Formal \"%s\" redeclaration.\n", line, name);
        exit(1);
    }

    if (isSymbolLibraryFunction(name)) {
        printf("Error at line %d: Shadowing library function.\n", line);
        exit(1);
    }

    entry = symtab_insertVariable(table, name, line, scope, FORMAL);

    return entry;
}

SymbolTableEntry*
parserUtil_handleFuncPrefix(char* name, unsigned int line) {
    SymbolTableEntry* entry;
    Expr* result;

    entry = handleFunctionDefinition(name, line);
    result = icode_getLvalueExpr(entry);

    quad_emit(funcstart_op, NULL, NULL, result, 0, line);

    SCOPE_ENTER();
    scopeStack_push(SCOPE_FUNCTION);
    scopeSpace_pushCurrentOffset();
    scopeSpace_enterScopeSpace();
    scopeSpace_resetFormatArgsOffset();
    
    return entry;
}

void
parserUtil_handleFuncArgs() {
    scopeSpace_enterScopeSpace();
    scopeSpace_resetLocalOffset();
}

unsigned
parserUtil_handleFuncbody() {
    unsigned totalLocals;

    totalLocals = scopeSpace_currentScopeOffset();
    
    scopeSpace_exitScopeSpace();

    return totalLocals;
}

SymbolTableEntry*
parserUtil_handleFuncdef(SymbolTableEntry* funcPrefix, unsigned totalLocals, unsigned int line) {
    Expr* result;

    symtab_setFunctionLocal(funcPrefix, totalLocals);

    scopeSpace_exitScopeSpace();
    scopeSpace_restoreLocalOffset();

    result = icode_getLvalueExpr(funcPrefix);

    quad_emit(funcend_op, NULL, NULL, result, 0, line);

    symtab_hide(table, scope);
    SCOPE_EXIT();
    scopeStack_pop();

    return funcPrefix;
}

char*
parserUtil_generateUnnamedFunctionName() {
    static unsigned int counter = 0;
    char buffer[32];

    snprintf(buffer, sizeof(buffer), "anon_func_%u", counter++);

    char* name = malloc(strlen(buffer) + 1);
    if (!name) {
        fprintf(stderr, "Error: memory allocation failed in generateUnnamedFunctionName.\n");
        exit(1);
    }

    strcpy(name, buffer);
    return name;
}

Expr*
parserUtil_handleLvalueIdentifierTableItem(Expr* lv, char* identifier, unsigned int line) {
    return member_item(lv, identifier, line);
}

Expr*
parserUtil_handleLvalueExprTableItem(Expr* lv, Expr* e, unsigned int line) {

    Expr* tableItem;

    lv = emit_iftableitem(lv, line);
    tableItem = icode_newExpr(tableitem_e);
    
    icode_setExprEntry(tableItem, icode_getExprEntry(lv));
    icode_setExprIndex(tableItem, e);

    return tableItem;
}

Expr*
parserUtil_handlePrimary(Expr* lv, unsigned int line) {
    Expr* e;
    e = emit_iftableitem(lv, line);
    return e;
}

Expr*
parserUtil_handlePrimaryFuncdef(SymbolTableEntry* entry) {
    Expr* primary;
    primary = icode_newExpr(programfunc_e);
    icode_setExprEntry(primary, entry);
    return primary;
}

Expr*
parserUtil_handleAssignExpr(Expr* lv, Expr* e, unsigned int line) {
    Expr* assignExpr;

    if (icode_getExprType(lv) == tableitem_e) {
        assignExpr = assignToTableItem(lv, e, line);
    }
    else {
        assignExpr = assignToLvalue(lv, e, line);
    }

    return assignExpr;
}

Expr*
parserUtil_newConstnumExpr(double i) {
    return icode_newConstNum(i);
}

Expr*
parserUtil_newBoolExpr(unsigned char bool) {
    return icode_newConstBoolean(bool);
}

Call*
parserUtil_handleMethodCall(char* identifier, Expr* elist) {
    Call* call;

    call = icode_newCall();

    icode_setEList(call, elist);
    icode_setMethod(call, 1);
    icode_setName(call, strdup(identifier));

    return call;
}

Expr*
parserUtil_handleCall(Expr* call, Expr* elist, unsigned int line) {
    return make_call(call, elist, line);
}

Expr*
parserUtil_handleCallSuffix(Expr* lv, Call* callsuffix, unsigned int line) {
    
    Expr* call;
    Expr* elist;

    lv = emit_iftableitem(lv, line);

    if (icode_getMethod(callsuffix)) {
        elist = icode_getElist(callsuffix);
        elist = icode_insertFirst(elist, lv);
        icode_setEList(callsuffix, elist);
        lv = emit_iftableitem(member_item(lv, icode_getName(callsuffix),line), line);
    }
    
    elist = icode_getElist(callsuffix);
    call = make_call(lv, elist, line);

    return call;
}

Call*
parserUtil_handleNormCall(Expr* elist) {
    Call* call;

    call = icode_newCall();
    icode_setEList(call, elist);
    icode_setMethod(call, 0);
    icode_setName(call, NULL);

    return call;
}

Expr*
parserUtil_handleCallFuncdef(SymbolTableEntry* funcdef, Expr* elist, unsigned int line) {
    Expr* func;

    func = icode_newExpr(programfunc_e);
    icode_setExprEntry(func, funcdef);
    
    return make_call(func, elist, line);
}

Expr*
parserUtil_handleElist(Expr* elist, Expr* e) {
    Expr* tmp;

    tmp = elist;
    while(icode_getExprNext(tmp)) {
        tmp = icode_getExprNext(tmp);
    }
    icode_setExprNext(tmp, e);

    return elist;
}

Expr*
parserUtil_handleUminusExpr(Expr* e, unsigned int line) {
    Expr* term;

    icode_checkArithmetic(e, "unary minus");
    term = icode_newExpr(arithmexpr_e);
    icode_setExprEntry(term, newTemp(line));
    quad_emit(uminus_op, e, NULL, term, 0, line);

    return term;
}

Expr*
parserUtil_handleNotExpr(Expr* e, unsigned int line) {
    Expr* term;

    term = icode_newExpr(boolexpr_e);
    icode_setExprEntry(term, newTemp(line));
    quad_emit(not_op, e, NULL, term, 0, line);

    return term;
}

Expr*
parserUtil_handleLvalueIncrement(Expr* lv, unsigned int line) {

    Expr* term;
    Expr* val;

    icode_checkArithmetic(lv, "lvalue++");
    term = icode_newExpr(var_e);
    icode_setExprEntry(term, newTemp(line));

    if (icode_getExprType(lv) == tableitem_e) {
        val = emit_iftableitem(lv, line);
        quad_emit(assign_op, val, NULL, term, 0, line);
        quad_emit(add_op, val, icode_newConstNum(1), val, 0, line);
        quad_emit(tablesetelem_op, lv, icode_getExprIndex(lv), val, 0, line);
    }
    else {
        quad_emit(assign_op, lv, NULL, term, 0, line);
        quad_emit(add_op, lv, icode_newConstNum(1), lv, 0, line);
    }

    return term;
}

Expr*
parserUtil_handleIncrementLvalue(Expr* lv, unsigned int line) {
    Expr* term;

    icode_checkArithmetic(lv, "++lvalue");

    if (icode_getExprType(lv) == tableitem_e) {
        term = emit_iftableitem(lv, line);
        quad_emit(add_op, term, icode_newConstNum(1), term, 0, line);
        quad_emit(tablesetelem_op, lv, icode_getExprIndex(lv), term, 0, line);
    }
    else {
        quad_emit(add_op, lv, icode_newConstNum(1), lv, 0, line);
        term = icode_newExpr(arithmexpr_e);
        icode_setExprEntry(term, newTemp(line));
        quad_emit(assign_op, lv, NULL, term, 0, line);
    }

    return term;
}

Expr*
parserUtil_handleLvalueDecrement(Expr* lv, unsigned int line) {
    Expr* term;
    Expr* val;

    icode_checkArithmetic(lv, "lvalue--");
    term = icode_newExpr(var_e);
    icode_setExprEntry(term, newTemp(line));

    if (icode_getExprType(lv) == tableitem_e) {
        val = emit_iftableitem(lv, line);
        quad_emit(assign_op, val, NULL, term, 0, line);
        quad_emit(sub_op, val, icode_newConstNum(1), val, 0, line);
        quad_emit(tablesetelem_op, lv, icode_getExprIndex(lv), val, 0, line);
    }
    else {
        quad_emit(assign_op, lv, NULL, term, 0, line);
        quad_emit(sub_op, lv, icode_newConstNum(1), lv, 0, line);
    }

    return term;
}

Expr*
parserUtil_handleDecrementLvalue(Expr* lv, unsigned int line) {
    Expr* term;

    icode_checkArithmetic(lv, "--lvalue");

    if (icode_getExprType(lv) == tableitem_e) {
        term = emit_iftableitem(lv, line);
        quad_emit(sub_op, term, icode_newConstNum(1), term, 0, line);
        quad_emit(tablesetelem_op, lv, icode_getExprIndex(lv), term, 0, line);
    }
    else {
        quad_emit(sub_op, lv, icode_newConstNum(1), lv, 0, line);
        term = icode_newExpr(arithmexpr_e);
        icode_setExprEntry(term, newTemp(line));
        quad_emit(assign_op, lv, NULL, term, 0, line);
    }

    return term;
}

void
parserUtil_printSymbolTable() {
    symtab_printScopeTable(table);
}

/* ======================================== STATIC DEFINITIONS ======================================== */
static int
isSymbolLibraryFunction(const char* name) {
    for (int i = 0; i < NUM_LIBRARY_FUNCTIONS; i++) {
        if (strcmp(libraryFunctions[i], name) == 0) {
            return 1;
        }
    }
    return 0;
}

static void
insertLibraryFunctions() {
    for (int i = 0; i < NUM_LIBRARY_FUNCTIONS; i++) {
        symtab_insertFunction(table, libraryFunctions[i], 0, 0, LIBFUNC);
    }
}

SymbolTableEntry*
handleFunctionDefinition(const char* name, unsigned int line) {
    SymbolTableEntry* entry;

    entry = symtab_lookupInScopeTable(table, name, scope);

    if (entry && symtab_isEntryActive(entry)) {
        printf("Error at line %d: Function name redefinition.\n", line);
        exit(1);
    }

    if (isSymbolLibraryFunction(name)) {
        printf("Error at line %d: Shadowing library function.\n", line);
        exit(1);
    }

    entry = symtab_insertFunction(table, name, line, scope, USERFUNC);

    return entry;
}

void
reportInaccessibleVariable(unsigned int line1, unsigned int line2, const char* name) {
    fprintf(stderr, "Error at line %u: Variable \"%s\" at line %d is inaccessible.\n", line1, name, line2);
    exit(1);
}

char*
newTempName() {
    char* name;

    name = malloc(16);

    if (!name) {
        printf("Error allocating memory for temp var name.\n");
        exit(1);
    }

    sprintf(name, "_t%u", tempCounter++);
    return name;
}

void
resetTemp() {
    tempCounter = 0;
}

SymbolTableEntry*
newTemp(unsigned int line) {
    char* name;
    SymbolTableEntry* entry;

    name = newTempName();

    SymbolType type = (scope == 0) ? GLOBAL : LOCAL_T;
    entry = symtab_insertVariable(table, name, line, scope, type);

    return entry;
}

Expr*
emit_iftableitem(Expr* e, unsigned int line) {

    if (icode_getExprType(e) != tableitem_e) {
        return e;
    }

    Expr* result;
    Expr* index;
    SymbolTableEntry* entry;

    result = icode_newExpr(var_e);
    entry = newTemp(line);
    index = icode_getExprIndex(e);    

    icode_setExprEntry(result, entry);
    quad_emit(tablegetelem_op, e, index, result, 0, line);

    return result;
}

Expr*
make_call(Expr* lv, Expr* elist, unsigned int line) {
    Expr* func;
    Expr* result;
    Expr* reversed_elist;

    func = emit_iftableitem(lv, line);
    reversed_elist = icode_reverseExprList(elist);

    while(reversed_elist) {
        quad_emit(param_op, reversed_elist, NULL, NULL, 0, line);
        reversed_elist = icode_getExprNext(reversed_elist);
    }

    quad_emit(call_op, func, NULL, NULL, 0, line);
    result = icode_newExpr(var_e);
    icode_setExprEntry(result, newTemp(line));
    quad_emit(getretval_op, NULL, NULL, result, 0, line);

    return result;
}

Expr*
member_item(Expr* lv, char* identifier, unsigned int line) {
    Expr* tableItem;

    lv = emit_iftableitem(lv, line);
    tableItem = icode_newExpr(tableitem_e);

    icode_setExprEntry(tableItem, icode_getExprEntry(lv));
    icode_setExprIndex(tableItem, icode_newConstString(identifier));

    return tableItem;
}

Expr*
assignToTableItem(Expr* lv, Expr* e, unsigned int line) {
    Expr* index;
    Expr* assignExpr;

    index = icode_getExprIndex(lv);

    quad_emit(tablesetelem_op, lv, index, e, 0, line);
    assignExpr = emit_iftableitem(lv, line);
    icode_setExprType(assignExpr, assignexpr_e);

    return assignExpr;
}

Expr*
assignToLvalue(Expr* lv, Expr* e, unsigned int line) {
    Expr* assignExpr;

    quad_emit(assign_op, e, NULL, lv, 0, line);
    assignExpr = icode_newExpr(assignexpr_e);
    icode_setExprEntry(assignExpr, newTemp(line));
    quad_emit(assign_op, lv, NULL, assignExpr, 0, line);

    return assignExpr;
}