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

SymbolTableEntry*
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

    return entry;
}

SymbolTableEntry*
parserUtil_habdleGlobalLookup(const char* name, unsigned int line) {
    SymbolTableEntry* entry;

    entry = symtab_lookupInScopeTable(table, name, 0);

    if (!entry) {
        printf("Error at line %d: Global \"%s\" not found.\n", line, name);
        exit(1);
    }

    return entry;
}

SymbolTableEntry*
parserUtil_handleNamedFunction(const char* name, unsigned int line) {
    return handleFunctionDefinition(name, line);
}

SymbolTableEntry*
parserUtil_handleUnamedFunction(unsigned int line) {
    return handleFunctionDefinition(parserUtil_generateUnnamedFunctionName(), line);
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
parserUtil_handleIdentifier(const char* name, unsigned int line) {
    SymbolTableEntry* entry;

    for (int currentScope = scope; currentScope >= 1; currentScope--) {
        entry = symtab_lookupInScopeTable(table, name, currentScope);

        if (entry && symtab_isEntryActive(entry)) {
            if (currentScope == scope) {
                return entry;
            }
            else if (scopeStack_isAccessible(currentScope, scope)) {
                return entry;
            }
            else {
                reportInaccessibleVariable(line, symtab_getEntryLine(entry), name);
            }
        }
    }

    entry = symtab_lookupInScopeTable(table, name, 0);

    if (entry) {
        return entry;
    }

    ScopeType type = (scope == 0) ? GLOBAL : LOCAL_T;
    return symtab_insertVariable(table, name, line, scope, type);
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