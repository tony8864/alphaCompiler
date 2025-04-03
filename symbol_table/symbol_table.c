#include "symbol_table.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define SCOPE_TABLE_SIZE 200
#define COLLISION_TABLE_SIZE 200

typedef struct Function {
    const char* name;
    unsigned int line;
    unsigned int scope;
} Function;

typedef struct Variable {
    const char* name;
    unsigned int line;
    unsigned int scope;
} Variable;

typedef struct SymbolTableEntry {
    int isActive;
    union {
        Variable* varValue;
        Function* funcValue;
    } value;

    SymbolType type;
    struct SymbolTableEntry* collisionNext;
    struct SymbolTableEntry* scopeNext;
} SymbolTableEntry;

typedef struct SymbolTable {
    SymbolTableEntry* collisionTable[COLLISION_TABLE_SIZE];
    SymbolTableEntry* scopeTable[SCOPE_TABLE_SIZE];
} SymbolTable;

/* ======================================== STATIC DECLARATIONS ======================================== */
void
insertEntryInCollisionTable(SymbolTable* table, SymbolTableEntry* entry, unsigned int hashValue);

void
insertEntryInScopeTable(SymbolTable* table, SymbolTableEntry* entry, unsigned int scope);

Variable*
initializeVariable(const char* name, unsigned int line, unsigned int scope);

Function*
initializeFunction(const char* name, unsigned int line, unsigned int scope);

SymbolTableEntry*
initializeVariableEntry(Variable* variable, SymbolType type);

SymbolTableEntry*
initializeFunctionEntry(Function* function, SymbolType type);

int
isFunctionSymbol(SymbolType symbol);

int
isVariableSymbol(SymbolType symbol);

const char*
getEntryName(SymbolTableEntry* entry);

static unsigned int
hash(const char* key);

/* ======================================== IMPLEMENTATION ======================================== */
SymbolTable*
symtab_initialize() {
    SymbolTable* symTable;

    symTable = malloc(sizeof(SymbolTable));

    if (!symTable) {
        printf("Error allocating memory for symbol table.\n");
        exit(1);
    }

    return symTable;
}

void
symtab_insertVariable(SymbolTable* table, const char* name, unsigned int line, unsigned int scope, SymbolType type) {
    Variable* variable;
    SymbolTableEntry* entry;

    variable = initializeVariable(name, line, scope);
    entry = initializeVariableEntry(variable, type);

    insertEntryInCollisionTable(table, entry, hash(name));
    insertEntryInScopeTable(table, entry, scope);
}

void
symtab_insertFunction(SymbolTable* table, const char* name, unsigned int line, unsigned int scope, SymbolType type) {
    Function* function;
    SymbolTableEntry* entry;

    function = initializeFunction(name, line, scope);
    entry = initializeFunctionEntry(function, type);

    insertEntryInCollisionTable(table, entry, hash(name));
    insertEntryInScopeTable(table, entry, scope);
}

SymbolTableEntry*
symtab_lookupInCollisionTable(SymbolTable* table, const char* name) {
    SymbolTableEntry* entry;

    entry = table->collisionTable[hash(name)];

    while (entry != NULL && strcmp(getEntryName(entry), name) != 0) {
        entry = entry->collisionNext;
    }
    
    return entry;
}

SymbolTableEntry*
symtab_lookupInScopeTable(SymbolTable* table, const char* name, unsigned int scope) {
    SymbolTableEntry* entry;

    entry = table->scopeTable[scope];

    while (entry != NULL && strcmp(getEntryName(entry), name) != 0) {
        entry = entry->scopeNext;
    }
    
    return entry;
}

void
symtab_printCollisionTable(SymbolTable* table) {
    SymbolTableEntry* head;

    for (int i = 0; i < COLLISION_TABLE_SIZE; i++) {
        head = table->collisionTable[i];
        if (head) {
            printf("Collision table: %d\n", i);
            printf("%-10s %-10s %-10s %-10s\n", "Name", "Line", "Scope", "Active");
            while (head) {
                Variable* variable = head->value.varValue;
                printf("%-10s %-10d %-10d %-10d\n", variable->name, variable->line, variable->scope, head->isActive);
                head = head->collisionNext;
            }
        }
    }
}

void
symtab_hide(SymbolTable* table, unsigned int scope) {
    SymbolTableEntry* entry;

    entry = table->scopeTable[scope];

    while (entry) {
        entry->isActive = 0;
        entry = entry->scopeNext;
    }
}

void
symtab_printScopeTable(SymbolTable* table) {
    SymbolTableEntry* head;

    for (int i = 0; i < SCOPE_TABLE_SIZE; i++) {
        head = table->scopeTable[i];
        if (head) {
            printf("Scope table: %d\n", i);
            printf("%-10s %-10s %-10s %-10s\n", "Name", "Line", "Scope", "Active");
            while (head) {
                Variable* variable = head->value.varValue;
                printf("%-10s %-10d %-10d %-10d\n", variable->name, variable->line, variable->scope, head->isActive);
                head = head->scopeNext;
            }
        }
    }
}

/* ======================================== STATIC DEFINITIONS ======================================== */
void
insertEntryInCollisionTable(SymbolTable* table, SymbolTableEntry* entry, unsigned int hashValue) {
    SymbolTableEntry* head;

    head = table->collisionTable[hashValue];

    if (head) {
        entry->collisionNext = head;
    }

    table->collisionTable[hashValue] = entry;
}

void
insertEntryInScopeTable(SymbolTable* table, SymbolTableEntry* entry, unsigned int scope) {
    SymbolTableEntry* head;

    head = table->scopeTable[scope];

    if (head) {
        entry->scopeNext = head;
    }

    table->scopeTable[scope] = entry;
}

Variable*
initializeVariable(const char* name, unsigned int line, unsigned int scope) {
    Variable* variable;
    
    variable = malloc(sizeof(Variable));

    if (!variable) {
        printf("Error allocating memory for variable symbol.\n");
        exit(1);
    }

    variable->name = name;
    variable->line = line;
    variable->scope = scope;

    return variable;
}

Function*
initializeFunction(const char* name, unsigned int line, unsigned int scope) {
    Function* function;

    function = malloc(sizeof(Function));

    if (!function) {
        printf("Error allocating memory for function symbol.\n");
        exit(1);
    }

    function->name = name;
    function->line = line;
    function->scope = scope;

    return function;
}

SymbolTableEntry*
initializeVariableEntry(Variable* variable, SymbolType type) {
    SymbolTableEntry* entry;

    entry = malloc(sizeof(SymbolTableEntry));

    if (!entry) {
        printf("Error allocating memory for variable entry.\n");
        exit(1);
    }

    if (!isVariableSymbol(type)) {
        printf("The type is not a valid Variable type.\n");
        exit(1);
    }

    entry->isActive = 1;
    entry->value.varValue = variable;
    entry->collisionNext = NULL;
    entry->scopeNext = NULL;
    entry->type = type;

    return entry;
}

SymbolTableEntry*
initializeFunctionEntry(Function* function, SymbolType type) {
    SymbolTableEntry* entry;

    entry = malloc(sizeof(SymbolTableEntry));

    if (!entry) {
        printf("Error allocating memory for function entry.\n");
        exit(1);
    }

    if (!isFunctionSymbol(type)) {
        printf("The type is not a valid Function type.\n");
        exit(1);
    }

    entry->isActive = 1;
    entry->value.funcValue = function;
    entry->collisionNext = NULL;
    entry->scopeNext = NULL;
    entry->type = type;

    return entry;
}

int
isFunctionSymbol(SymbolType symbol) {
    if (symbol == USERFUNC || symbol == LIBFUNC) {
        return 1;
    }
    return 0;
}

int
isVariableSymbol(SymbolType symbol) {
    if (symbol == LOCAL || symbol == GLOBAL || symbol == FORMAL) {
        return 1;
    }
    return 0;
}

const char*
getEntryName(SymbolTableEntry* entry) {
    if (entry->type == USERFUNC || entry->type == LIBFUNC) {
        return entry->value.funcValue->name;
    }
    return entry->value.varValue->name;
}

static unsigned int
hash(const char* key) {
    return strlen(key);
}
