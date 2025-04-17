#include "../scope_space/scope_space.h"
#include "symbol_table.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>

#define SCOPE_TABLE_SIZE 200
#define COLLISION_TABLE_SIZE 200

typedef struct Function {
    const char* name;
    unsigned int line;
    unsigned int scope;
    unsigned totalLocals;
} Function;

typedef struct Variable {
    const char* name;
    unsigned int line;
    ScopeSpaceType space;
    unsigned offset;
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
isFunctionSymbol(SymbolType type);

int
isVariableSymbol(SymbolType type);

char*
getStringFunctionType(SymbolType type);

char*
getStringVariableType(SymbolType type);

char*
getSpaceStringType(ScopeSpaceType type);

const char*
getEntryName(SymbolTableEntry* entry);

static uint32_t 
hash(const char *key);

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

SymbolTableEntry*
symtab_insertVariable(SymbolTable* table, const char* name, unsigned int line, unsigned int scope, SymbolType type) {
    Variable* variable;
    SymbolTableEntry* entry;

    variable = initializeVariable(name, line, scope);
    entry = initializeVariableEntry(variable, type);

    insertEntryInCollisionTable(table, entry, hash(name));
    insertEntryInScopeTable(table, entry, scope);

    return entry;
}

SymbolTableEntry*
symtab_insertFunction(SymbolTable* table, const char* name, unsigned int line, unsigned int scope, SymbolType type) {
    Function* function;
    SymbolTableEntry* entry;

    function = initializeFunction(name, line, scope);
    entry = initializeFunctionEntry(function, type);

    insertEntryInCollisionTable(table, entry, hash(name));
    insertEntryInScopeTable(table, entry, scope);

    return entry;
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
symtab_hide(SymbolTable* table, unsigned int scope) {
    SymbolTableEntry* entry;

    entry = table->scopeTable[scope];

    while (entry) {
        entry->isActive = 0;
        entry = entry->scopeNext;
    }
}

int
symtab_isEntryActive(SymbolTableEntry* entry) {
    return entry->isActive;
}

unsigned int
symtab_getEntryLine(SymbolTableEntry* entry) {
    if (isVariableSymbol(entry->type)) {
        return entry->value.varValue->line;
    }
    else {
        return entry->value.funcValue->line;
    }
}

unsigned int
symtab_getEntryScope(SymbolTableEntry* entry) {
    if (isVariableSymbol(entry->type)) {
        return entry->value.varValue->scope;
    }
    else {
        return entry->value.funcValue->scope;
    }
}

SymbolType
symtab_getEntryType(SymbolTableEntry* entry) {
    return entry->type;
}

void
symtab_setFunctionLocal(SymbolTableEntry* entry, unsigned totalLocals) {
    if (isFunctionSymbol(entry->type)) {
        entry->value.funcValue->totalLocals = totalLocals;
    }
    else {
        printf("Cannot set function local to variable symbol.\n");
        exit(1);
    }
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
symtab_printScopeTable(SymbolTable* table) {
    SymbolTableEntry* entry;

    for (int i = 0; i < SCOPE_TABLE_SIZE; i++) {
        entry = table->scopeTable[i];
        if (entry) {
            printf("---------------------- Scope %d ----------------------\n", i);
            while (entry) {
                
                if (isFunctionSymbol(entry->type)) {
                    Function* symbol = entry->value.funcValue;

                    char nameStr[30];
                    char lineStr[12];
                    char scopeStr[12];
                    char funcType[12];
                    char totalLocalStr[20];
                    
                    snprintf(nameStr, sizeof(nameStr), "\"%s\"", symbol->name);
                    snprintf(lineStr, sizeof(lineStr), "(line %d)", symbol->line);
                    snprintf(scopeStr, sizeof(scopeStr), "(scope %d)", symbol->scope);
                    snprintf(funcType, sizeof(funcType), "[%s]", getStringFunctionType(entry->type));
                    snprintf(totalLocalStr, sizeof(totalLocalStr), "(total locals %d)", symbol->totalLocals);
                    
                    printf("%-20s %-10s %-10s %-10s %-10s\n", nameStr, funcType, lineStr, scopeStr, totalLocalStr);
                }
                else {
                    Variable* symbol = entry->value.varValue;
    
                    char nameStr[30];
                    char lineStr[12];
                    char scopeStr[12];
                    char spaceStr[40];
                    char offsetStr[12];
                    char varType[12];
                    
                    snprintf(nameStr, sizeof(nameStr), "\"%s\"", symbol->name);
                    snprintf(lineStr, sizeof(lineStr), "(line %d)", symbol->line);
                    snprintf(scopeStr, sizeof(scopeStr), "(scope %d)", symbol->scope);
                    snprintf(spaceStr, sizeof(spaceStr), "(space %s)", getSpaceStringType(symbol->space));
                    snprintf(offsetStr, sizeof(offsetStr), "(offset %d)", symbol->offset);
                    snprintf(varType, sizeof(varType), "[%s]", getStringVariableType(entry->type));

                    printf("%-20s %-10s %-10s %-10s %-20s %-10s\n", nameStr, varType, lineStr, scopeStr, spaceStr, offsetStr);
                }
                entry = entry->scopeNext;
            }
        }
    }
}

/* ======================================== STATIC DEFINITIONS ======================================== */
void
insertEntryInCollisionTable(SymbolTable* table, SymbolTableEntry* entry, unsigned int hashValue) {
    SymbolTableEntry* head = table->collisionTable[hashValue];

    if (!head) {
        table->collisionTable[hashValue] = entry;
    } else {
        while (head->collisionNext) {
            head = head->collisionNext;
        }
        head->collisionNext = entry;
    }

    entry->collisionNext = NULL;
}

void
insertEntryInScopeTable(SymbolTable* table, SymbolTableEntry* entry, unsigned int scope) {
    SymbolTableEntry* head = table->scopeTable[scope];

    if (!head) {
        table->scopeTable[scope] = entry;
    } else {
        while (head->scopeNext) {
            head = head->scopeNext;
        }
        head->scopeNext = entry;
    }

    entry->scopeNext = NULL;
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
    variable->space = scopeSpace_currentScope();
    variable->offset = scopeSpace_currentScopeOffset();

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
    function->totalLocals = 0;

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

    scopeSpace_incrementCurrentScopeOffset();

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
isFunctionSymbol(SymbolType type) {
    if (type == USERFUNC || type == LIBFUNC) {
        return 1;
    }
    return 0;
}

int
isVariableSymbol(SymbolType type) {
    if (type == LOCAL_T || type == GLOBAL || type == FORMAL) {
        return 1;
    }
    return 0;
}

char*
getStringFunctionType(SymbolType type) {
    if (type == USERFUNC) {
        return "USERFUNC";
    }
    else if (type == LIBFUNC) {
        return "LIBFUNC";
    }
    else {
        printf("Unrecognized function type.\n");
        exit(1);
    }
}

char*
getStringVariableType(SymbolType type) {
    if (type == LOCAL_T) {
        return "LOCAL";
    }
    else if (type == GLOBAL) {
        return "GLOBAL";
    }
    else if (type == FORMAL) {
        return "FORMAL";
    }
    else {
        printf("Unrecognized variable type.\n");
        exit(1);
    }
}

char*
getSpaceStringType(ScopeSpaceType type) {
    if (type == PROGRAMVAR) {
        return "PROGRAMVAR";
    }
    else if (type == FORMALARG) {
        return "FORMALARG";
    }
    else if (type == FUNCTIONLOCAL) {
        return "FUNCTIONLOCAL";
    }
    else {
        printf("Unrecognized scope type.\n");
        exit(1);
    }
}

const char*
getEntryName(SymbolTableEntry* entry) {
    if (entry->type == USERFUNC || entry->type == LIBFUNC) {
        return entry->value.funcValue->name;
    }
    return entry->value.varValue->name;
}

static uint32_t 
hash(const char *key) {
    size_t length = strlen(key);
    uint32_t hash = 2166136261u;
    for (size_t i = 0; i < length; ++i) {
        hash ^= (uint8_t)key[i];
        hash *= 16777619u;
    }
    return hash % COLLISION_TABLE_SIZE;
}