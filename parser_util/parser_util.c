#include "../scope_stack/scope_stack.h"
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

char*
generateUnnamedFunctionName();

/* ======================================== IMPLEMENTATION ======================================== */
void
parserUtil_initialize() {
    table = symtab_initialize();
    insertLibraryFunctions();
    scopeStack_initialize();
}

void
parserUtil_printSymbolTable() {
    symtab_printScopeTable(table);
}

void
parserUtil_cleanup() {
    symtab_printScopeTable(table);
    scopeStack_cleanup();
}

void
parserUtil_handleBlockEntrance() {
    SCOPE_ENTER();
    scopeStack_push(SCOPE_BLOCK);
}

void
parserUtil_handleBlockExit() {
    SCOPE_EXIT();
    scopeStack_pop();
    
}

void
parserUtil_handleFuncBlockEntrance() {
    SCOPE_ENTER();
    scopeStack_push(SCOPE_FUNCTION);
}

void
parserUtil_handleFuncBlockExit() {
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
        
        if (scope == 0) {
            entry = symtab_insertVariable(table, name, line, scope, GLOBAL);
        }
        else {
            entry = symtab_insertVariable(table, name, line, scope, LOCAL_T);
        }
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
    return handleFunctionDefinition(generateUnnamedFunctionName(), line);
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
    unsigned int currentScope;

    if ((entry = symtab_lookupInScopeTable(table, name, 0)) != NULL) {
        return entry;
    }

    for (currentScope = scope; currentScope >= 1; currentScope--) {
        entry = symtab_lookupInScopeTable(table, name, currentScope);

        if (entry) {
            if (scopeStack_isAccessible(currentScope, scope)) {
                return entry;
            }
            else {
                printf("Error at line %d: Variable \"%s\" is inaccessible.\n", line, name);
                exit(1);
            }
        }
    }

    entry = (scope == 0) ?
            symtab_insertVariable(table, name, line, scope, GLOBAL) : 
            symtab_insertVariable(table, name, line, scope, LOCAL_T);

    return entry;
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

char*
generateUnnamedFunctionName() {
    static unsigned int counter = 0;
    char buffer[32]; // Enough for "anon_func_" + digits

    snprintf(buffer, sizeof(buffer), "anon_func_%u", counter++);

    // Dynamically allocate and return the name
    char* name = malloc(strlen(buffer) + 1);
    if (!name) {
        fprintf(stderr, "Error: memory allocation failed in generateUnnamedFunctionName.\n");
        exit(1);
    }

    strcpy(name, buffer);
    return name;
}