#ifndef SYM_TAB_H
#define SYM_TAB_H

typedef struct SymbolTableEntry SymbolTableEntry;
typedef struct SymbolTable SymbolTable;
typedef struct Variable Variable;
typedef struct Function Function;

typedef enum { GLOBAL, LOCAL, FORMAL, USERFUNC, LIBFUNC } SymbolType;

SymbolTable*
symtab_initialize();

void
symtab_insertVariable(SymbolTable* table, const char* name, unsigned int line, unsigned int scope, SymbolType type);

void
symtab_insertFunction(SymbolTable* table, const char* name, unsigned int line, unsigned int scope, SymbolType type);

SymbolTableEntry*
symtab_lookupInCollisionTable(SymbolTable* table, const char* name);

SymbolTableEntry*
symtab_lookupInScopeTable(SymbolTable* table, const char* name, unsigned int scope);

void
symtab_hide(SymbolTable* table, unsigned int scope);

void
symtab_printCollisionTable(SymbolTable* table);

void
symtab_printScopeTable(SymbolTable* table);

#endif
