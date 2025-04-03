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

/* ======================================== STATIC DECLARATIONS ======================================== */
int
isSymbolLibraryFunction(const char* name);

/* ======================================== IMPLEMENTATION ======================================== */
void
parserUtil_insertLibraryFunctions(SymbolTable* table) {
    for (int i = 0; i < NUM_LIBRARY_FUNCTIONS; i++) {
        symtab_insertFunction(table, libraryFunctions[i], 0, 0, LIBFUNC);
    }
}


SymbolTableEntry*
parserUtil_handleLocalIdentifier(SymbolTable* table, const char* name, unsigned int line, unsigned int scope, SymbolType type) {
    SymbolTableEntry* entry;

    entry = symtab_lookupInScopeTable(table, name, scope);

    if (isSymbolLibraryFunction(name)) {
        printf("Error at line %d: Shadowing library function.\n", line);
        exit(1);
    }

    if (!entry) {
        entry = symtab_insertVariable(table, name, line, scope, type);
    }

    return entry;
}

/* ======================================== STATIC DEFINITIONS ======================================== */
int
isSymbolLibraryFunction(const char* name) {
    for (int i = 0; i < NUM_LIBRARY_FUNCTIONS; i++) {
        if (strcmp(libraryFunctions[i], name) == 0) {
            return 1;
        }
    }
    return 0;
}