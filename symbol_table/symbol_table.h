#ifndef SYM_TAB_H
#define SYM_TAB_H

typedef struct SymbolTableEntry SymbolTableEntry;
typedef struct SymbolTable SymbolTable;
typedef struct Variable Variable;
typedef struct Function Function;

typedef enum { GLOBAL, LOCAL_T, FORMAL, USERFUNC, LIBFUNC } SymbolType;

SymbolTable*
symtab_initialize();

SymbolTableEntry*
symtab_insertVariable(SymbolTable* table, const char* name, unsigned int line, unsigned int scope, SymbolType type);

SymbolTableEntry*
symtab_insertFunction(SymbolTable* table, const char* name, unsigned int line, unsigned int scope, SymbolType type);

SymbolTableEntry*
symtab_lookupInCollisionTable(SymbolTable* table, const char* name);

SymbolTableEntry*
symtab_lookupInScopeTable(SymbolTable* table, const char* name, unsigned int scope);

void
symtab_hide(SymbolTable* table, unsigned int scope);

int
symtab_isEntryActive(SymbolTableEntry* entry);

const char*
symtab_getEntryName(SymbolTableEntry* entry);

unsigned int
symtab_getEntryLine(SymbolTableEntry* entry);

unsigned int
symtab_getEntryScope(SymbolTableEntry* entry);

SymbolType
symtab_getEntryType(SymbolTableEntry* entry);

void
symtab_setFunctionLocal(SymbolTableEntry* entry, unsigned totalLocals);

void
symtab_printCollisionTable(SymbolTable* table);

void
symtab_printScopeTable(SymbolTable* table);

#endif
