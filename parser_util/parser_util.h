#ifndef PARSER_UTIL_H
#define PARSER_UTIL_H

#include "../symbol_table/symbol_table.h"

void
parserUtil_insertLibraryFunctions(SymbolTable* table);

SymbolTableEntry*
parserUtil_handleLocalIdentifier(SymbolTable* table, const char* name, unsigned int line, unsigned int scope, SymbolType type);

SymbolTableEntry*
parserUtil_handleNamedFunction(SymbolTable* table, const char* name, unsigned int line, unsigned int scope, SymbolType type);

SymbolTableEntry*
parserUtil_habdleGlobalLookup(SymbolTable* table, const char* name, unsigned int line);

#endif