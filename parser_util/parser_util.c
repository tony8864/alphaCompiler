#include "../scope_space/scope_space.h"
#include "../scope_stack/scope_stack.h"
#include "../lc_stack/lc_stack.h"
#include "../quad/quad.h"
#include "../tcode/tcode.h"
#include "parser_util.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

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
static unsigned int funcCounter = 0;

#define SCOPE_ENTER()   (scope++)
#define SCOPE_EXIT()    (scope--)

#define FUNC_ENTER()    (funcCounter++)
#define FUNC_EXIT()     (funcCounter--)

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

void
reportLvalueFunction(Expr* lv, unsigned int line);

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

unsigned
isInsideFunction();

Expr*
handleConstnumArithmExpression(Expr* e1, Expr* e2, IOPCodeType op, unsigned int line);

Expr*
handleArithmeticExpression(Expr* e1, Expr* e2, IOPCodeType type, unsigned int line);

Expr*
handleConstNumRelationalExpression(Expr* e1, Expr* e2, IOPCodeType op, unsigned int line);

Expr*
handleRelationalExpression(Expr* e1, Expr* e2, IOPCodeType op, unsigned int line);

void
handlePrints();

void
handleCleanups();

/* ======================================== IMPLEMENTATION ======================================== */
void
parserUtil_initialize() {
    table = symtab_initialize();
  
    insertLibraryFunctions();

    scopeStack_initialize();
    scopeSpace_initialize();

    lcStack_initialize();
    lcStack_pushLoopCounter();

    tcode_initialize();

    quad_emit(jump_op, NULL, NULL, NULL, 1, 0);
}

void
parserUtil_finalize() {
    tcode_generateInstructions();
    handlePrints();
    quad_writeQuadsToFile("quads.txt");
    handleCleanups();
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
    resetTemp();
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

    SymbolType type = (scope == 0) ? GLOBAL : LOCAL_T;
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
    Expr* arg;

    entry = handleFunctionDefinition(name, line);
    arg = icode_getLvalueExpr(entry);

    quad_emit(jump_op, NULL, NULL, NULL, 0, line);
    quad_emit(funcstart_op, arg, NULL, NULL, 0, line);

    SCOPE_ENTER();
    FUNC_ENTER();
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
parserUtil_handleFuncdef(SymbolTableEntry* funcPrefix, unsigned int M, unsigned totalLocals, unsigned int line) {
    Expr* arg;

    symtab_setFunctionLocal(funcPrefix, totalLocals);

    scopeSpace_exitScopeSpace();
    scopeSpace_restoreLocalOffset();

    arg = icode_getLvalueExpr(funcPrefix);

    quad_emit(funcend_op, arg, NULL, NULL, 0, line);
    quad_patchLabel(M - 2, quad_nextQuadLabel());

    symtab_hide(table, scope);
    SCOPE_EXIT();
    FUNC_EXIT();
    scopeStack_pop();

    return funcPrefix;
}

char*
parserUtil_generateUnnamedFunctionName() {
    static unsigned int counter = 0;
    char buffer[32];

    snprintf(buffer, sizeof(buffer), "_$func_%u", counter++);

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
    
    reportLvalueFunction(lv, line);

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
parserUtil_newConstString(char* str) {
    return icode_newConstString(str);
}

Expr*
parserUtil_newConstNil() {
    return icode_newExpr(nil_e);
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

    icode_checkArithmetic(e, "unary minus", line);
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

    icode_checkArithmetic(lv, "lvalue++", line);
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

    icode_checkArithmetic(lv, "++lvalue", line);

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

    icode_checkArithmetic(lv, "lvalue--", line);
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

    icode_checkArithmetic(lv, "--lvalue", line);

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

Expr*
parserUtil_handleMakeElistTable(Expr* elist, unsigned int line) {
    Expr* t;

    t = icode_newExpr(newtable_e);
    icode_setExprEntry(t, newTemp(line));
    quad_emit(tablecreate_op, t, NULL, NULL, 0, line);

    for (int i = 0; elist; elist = icode_getExprNext(elist)) {
        quad_emit(tablesetelem_op, t, icode_newConstNum(i++), elist, 0, line);
    }

    return t;
}

Indexed*
parserUtil_newIndexed(Expr* key, Expr* value) {
    return icode_newIndexedElem(key, value);
}

Indexed*
parserUtil_handleIndexed(Indexed* indexedList, Indexed* indexed) {
    Indexed* tmp;

    tmp = indexedList;
    while(icode_getIndexedNext(tmp)) {
        tmp = icode_getIndexedNext(tmp);
    }
    icode_setIndexedNext(tmp, indexed);
    return indexedList;
}

Expr*
parserUtil_handleMakeIndexedTable(Indexed* indexedList, unsigned int line) {
    Expr* t;
    Expr* key;
    Expr* value;

    t = icode_newExpr(newtable_e);
    icode_setExprEntry(t, newTemp(line));
    quad_emit(tablecreate_op, t, NULL, NULL, 0, line);

    while(indexedList) {
        key = icode_getIndexedKey(indexedList);
        value = icode_getIndexedValue(indexedList);
        quad_emit(tablesetelem_op, t, key, value, 0, line);
        indexedList = icode_getIndexedNext(indexedList);
    }

    return t;
}

Expr*
parserUtil_handleArithmeticExpr(Expr* expr1, Expr* expr2, IOPCodeType op, unsigned int line) {
    icode_checkArithmetic(expr1, "expr1", line);
    icode_checkArithmetic(expr2, "expr2", line);

    Expr* e;

    if (icode_getExprType(expr1) == constnum_e && icode_getExprType(expr2)) {
        e = handleConstnumArithmExpression(expr1, expr2, op, line);
    }
    else {
        e = handleArithmeticExpression(expr1, expr2, op, line);
    }

    return e;
}

Expr*
parserUtil_handleRelationalExpr(Expr* expr1, Expr* expr2, IOPCodeType op, unsigned int line) {

    icode_checkArithmetic(expr1, "expr1", line);
    icode_checkArithmetic(expr2, "expr2", line);

    Expr* e;

    if (icode_getExprType(expr1) == constnum_e && icode_getExprType(expr2)) {
        e = handleConstNumRelationalExpression(expr1, expr2, op, line);
    }
    else {
        e = handleRelationalExpression(expr1, expr2, op, line);
    }

    return e;
}

Expr*
parserUtil_handleBooleanExpr(Expr* expr1, Expr* expr2, IOPCodeType op, unsigned int line) {
    Expr* e;

    e = icode_newExpr(boolexpr_e);
    icode_setExprEntry(e, newTemp(line));
    quad_emit(op, expr1, expr2, e, 0, line);
    
    return e;
}

unsigned int
parserUtil_handleIfPrefix(Expr* expr, unsigned int line) {
    unsigned int ifPrefix;
    quad_emit(if_eq_op, expr, icode_newConstBoolean(1), NULL, quad_nextQuadLabel() + 2, line);
    ifPrefix = quad_nextQuadLabel();
    quad_emit(jump_op, NULL, NULL, NULL, 0, line);
    return ifPrefix;
}

unsigned int
parserUtil_handleElse(unsigned int line) {
    unsigned int elsePrefix;
    elsePrefix = quad_nextQuadLabel();
    quad_emit(jump_op, NULL, NULL, NULL, 0, line);
    return elsePrefix;
}

Statement*
parserUtil_handleIfElsePrefixStatement(unsigned int ifPrefix, Statement* stmt1, unsigned int elsePrefix, Statement* stmt2) {
    Statement* stmt;

    int newBreakList;
    int newContList;

    int breakList1 = 0, breakList2 = 0;
    int contList1 = 0, contList2 = 0;

    quad_patchLabel(ifPrefix, elsePrefix+1);
    quad_patchLabel(elsePrefix, quad_nextQuadLabel());

    stmt = icode_newStatement();

    if (stmt1 != NULL) {
        contList1 = icode_getContList(stmt1);
        breakList1 = icode_getBreakList(stmt1);
    }

    if (stmt2 != NULL) {
        contList2 = icode_getContList(stmt2);
        breakList2 = icode_getBreakList(stmt2);
    }

    newContList = quad_mergeList(contList1, contList2);
    newBreakList = quad_mergeList(breakList1, breakList2);

    quad_printList(newBreakList);

    icode_setContList(stmt, newContList);
    icode_setBreakList(stmt, newBreakList);

    return stmt;
}

void
parserUtil_handleIfPrefixStatement(unsigned int ifprefix) {
    quad_patchLabel(ifprefix, quad_nextQuadLabel());
}

unsigned int
parserUtil_handleWhileStart() {
    return quad_nextQuadLabel();
}

unsigned int
parserUtil_handleWhileCond(Expr* expr, unsigned int line) {
    unsigned int whileCond;
    quad_emit(if_eq_op, expr, icode_newConstBoolean(1), NULL, quad_nextQuadLabel()+2, line);
    whileCond = quad_nextQuadLabel();
    quad_emit(jump_op, NULL, NULL, NULL, 0, line);
    return whileCond;
}

void
parserUtil_handleWhileStatement(unsigned int whileStart, unsigned int whileCond, Statement* stmt, unsigned int line) {
    int contList = 0;
    int breakList = 0;

    quad_emit(jump_op, NULL, NULL, NULL, whileStart, line);
    quad_patchLabel(whileCond, quad_nextQuadLabel());

    if (stmt != NULL) {
        contList = icode_getContList(stmt);
        breakList = icode_getBreakList(stmt);
    }

    quad_patchList(contList, whileStart);
    quad_patchList(breakList, quad_nextQuadLabel());
}

void
parserUtil_handleForStatement(ForPrefix* forPrefix, unsigned int N1, unsigned int N2, Statement* stmt, unsigned int N3) {
    int contList = 0;
    int breakList = 0;

    quad_patchLabel(icode_getForPrefixEnter(forPrefix), N2 + 1);
    quad_patchLabel(N1, quad_nextQuadLabel());
    quad_patchLabel(N2, icode_getForPrefixTest(forPrefix));
    quad_patchLabel(N3, N1 + 1);

    if (stmt != NULL) {
        contList = icode_getContList(stmt);
        breakList = icode_getBreakList(stmt);
    }

    quad_patchList(contList, N1 + 2);
    quad_patchList(breakList, quad_nextQuadLabel());
}

ForPrefix*
parserUtil_handleForPrefix(unsigned int M, Expr* expr, unsigned int line) {
    ForPrefix* forPrefix;
    forPrefix = icode_newForPrefix(M, quad_nextQuadLabel());
    quad_emit(if_eq_op, expr, icode_newConstBoolean(1), NULL, 0, line);
    return forPrefix;
}

unsigned int
parserUtil_handleNrule(unsigned int line) {
    unsigned int N;
    N = quad_nextQuadLabel();
    quad_emit(jump_op, NULL, NULL, NULL, 0, line);
    return N;
}

unsigned int
parserUtil_handleMrule(unsigned int line) {
    return quad_nextQuadLabel();
}

Statement*
parserUtil_handleStatement(Statement* stmts, Statement* stmt) {
    Statement* newStmt;

    int newBreakList;
    int newContList;

    int breakList1 = 0, breakList2 = 0;
    int contList1 = 0, contList2 = 0;

    newStmt = icode_newStatement();

    if (stmts != NULL) {
        contList1 = icode_getContList(stmts);
        breakList1 = icode_getBreakList(stmts);
    }

    if (stmt != NULL) {
        contList2 = icode_getContList(stmt);
        breakList2 = icode_getBreakList(stmt);
    }

    newContList = quad_mergeList(contList1, contList2);
    newBreakList = quad_mergeList(breakList1, breakList2);
    
    icode_setContList(newStmt, newContList);
    icode_setBreakList(newStmt, newBreakList);
    
    return newStmt;
}

Statement*
parserUtil_handleContinue(unsigned int line) {

    if (!lcStack_isInsideLoop()) {
        printf("Error at line %d: Continue is not inside loop.\n", line);
        exit(1);
    }

    Statement* stmt;
    unsigned int nextQuad;

    stmt = icode_newStatement();
    nextQuad = quad_nextQuadLabel();

    icode_setContList(stmt, quad_newList(nextQuad));
    quad_emit(jump_op, NULL, NULL, NULL, 0, line);

    return stmt;
}

Statement*
parserUtil_handleBreak(unsigned int line) {

    if (!lcStack_isInsideLoop()) {
        printf("Error at line %d: Break is not inside loop.\n", line);
        exit(1);
    }

    Statement* stmt;
    unsigned int nextQuad;

    stmt = icode_newStatement();
    nextQuad = quad_nextQuadLabel();

    icode_setBreakList(stmt, quad_newList(nextQuad));
    quad_emit(jump_op, NULL, NULL, NULL, 0, line);

    return stmt;
}

Statement*
parserUtil_newStatement() {
    return icode_newStatement();
}

void
parserUtil_handleReturn(Expr* e, unsigned int line) {
    
    if (!isInsideFunction()) {
        printf("Error at line %u: Return is not inside a function.\n", line);
        exit(1);
    }

    quad_emit(ret_op, NULL, NULL, e, 0, line);
}

void*
parserUtil_handleGeneralStatement() {
    //resetTemp();
    return NULL;
}

void
parserUtil_handleLoopStart() {
    lcStack_incrementLoopCounter();
}

void
parserUtil_handleLoopEnd() {
    lcStack_decrementLoopCounter();
}

void
parserUtil_handleFuncBlockStart() {
    lcStack_pushLoopCounter();
}

void
parserUtil_handleFuncBlockEnd() {
    lcStack_popLoopCouter();
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
        symtab_insertFunction(table, libraryFunctions[i], 0, 0, 0, LIBFUNC);
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

    entry = symtab_insertFunction(table, name, line, scope, quad_nextQuadLabel(), USERFUNC);

    return entry;
}

void
reportInaccessibleVariable(unsigned int line1, unsigned int line2, const char* name) {
    fprintf(stderr, "Error at line %u: Variable \"%s\" at line %d is inaccessible.\n", line1, name, line2);
    exit(1);
}

void
reportLvalueFunction(Expr* lv, unsigned int line) {
    SymbolTableEntry* entry;
    SymbolType type;

    entry = icode_getExprEntry(lv);
    type = symtab_getEntryType(entry);

    if (type == LIBFUNC || type == USERFUNC) {
        printf("Error at line %u: Function is used as an l-value.\n", line);
        exit(1);
    }
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

unsigned
isInsideFunction() {
    assert(funcCounter >= 0);
    return funcCounter > 0;
}

Expr*
handleConstnumArithmExpression(Expr* e1, Expr* e2, IOPCodeType op, unsigned int line) {
    Expr* e;
    double num1, num2;
    
    num1 = icode_getNumConst(e1);
    num2 = icode_getNumConst(e2);

    switch (op) {
        case add_op: e = icode_newConstNum(num1 + num2);            break;
        case sub_op: e = icode_newConstNum(num1 - num2);            break;
        case mul_op: e = icode_newConstNum(num1 * num2);            break;
        case mod_op: e = icode_newConstNum((int)num1 % (int)num2);  break;
        case div_op:
            if (num2 == 0) {
                printf("Error at line %u: Cannot divide by 0.\n", line);
                exit(1);
            }
            e = icode_newConstNum(num1 / num2);
            break;
        default: 
            printf("Error at line %u: opcode not supported for const num expression.\n", line);
            exit(1);
    }

    icode_setExprEntry(e, newTemp(line));
    quad_emit(op, e1, e2, e, 0, line);

    return e;
}

Expr*
handleArithmeticExpression(Expr* e1, Expr* e2, IOPCodeType op, unsigned int line) {
    Expr* e;

    e = icode_newExpr(arithmexpr_e);
    icode_setExprEntry(e, newTemp(line));
    quad_emit(op, e1, e2, e, 0, line);

    return e;
}


Expr*
handleConstNumRelationalExpression(Expr* e1, Expr* e2, IOPCodeType op, unsigned int line) {
    Expr* e;
    double num1, num2;
    
    num1 = icode_getNumConst(e1);
    num2 = icode_getNumConst(e2);

    switch (op) {
        case if_greater_op:     e = icode_newConstBoolean(num1 > num2);  break;
        case if_greatereq_op:   e = icode_newConstBoolean(num1 >= num2); break;
        case if_less_op:        e = icode_newConstBoolean(num1 < num2);  break;
        case if_lesseq_op:      e = icode_newConstBoolean(num1 <= num2); break;
        case if_eq_op:          e = icode_newConstBoolean(num1 == num2); break;
        case if_noteq_op:       e = icode_newConstBoolean(num1 != num2); break;
        default:
            printf("Error at line %u: opcode not supported for const num expression.\n", line);
            exit(1);
    }

    icode_setExprEntry(e, newTemp(line));
    quad_emit(op, e1, e2, e, 0, line);

    return e;
}

Expr*
handleRelationalExpression(Expr* e1, Expr* e2, IOPCodeType op, unsigned int line) {
    Expr* e;

    e = icode_newExpr(boolexpr_e);
    icode_setExprEntry(e, newTemp(line));

    quad_emit(op, e1, e2, NULL, quad_nextQuadLabel()+3, line);
    quad_emit(assign_op, icode_newConstBoolean(0), NULL, e, 0, line);
    quad_emit(jump_op, NULL, NULL, NULL, quad_nextQuadLabel()+2, line);
    quad_emit(assign_op, icode_newConstBoolean(1), NULL, e, 0, line);

    return e;
}

void
handlePrints() {
    symtab_printScopeTable(table);
    scopeStack_print();
    quad_printQuads();
    tcode_printUserFuncs();
    tcode_printNamedLibs();
    tcode_printStringConsts();
    tcode_printNumConsts();
    tcode_printInstructions();
}

void
handleCleanups() {
    scopeStack_cleanup();
    scopeSpace_cleanup();
    lcStack_cleanup();
}