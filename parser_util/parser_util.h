#ifndef PARSER_UTIL_H
#define PARSER_UTIL_H

#include "../symbol_table/symbol_table.h"

void
parserUtil_initialize();

void
parserUtil_cleanup();

void
parserUtil_printSymbolTable();

void
parserUtil_handleBlockEntrance();

void
parserUtil_handleBlockExit();

SymbolTableEntry*
parserUtil_handleLocalIdentifier(const char* name, unsigned int line);

SymbolTableEntry*
parserUtil_handleNamedFunction(const char* name, unsigned int line);

SymbolTableEntry*
parserUtil_handleUnamedFunction(unsigned int line);

SymbolTableEntry*
parserUtil_handleFormalArgument(const char* name, unsigned int line);

SymbolTableEntry*
parserUtil_handleIdentifier(const char* name, unsigned int line);

SymbolTableEntry*
parserUtil_habdleGlobalLookup(const char* name, unsigned int line);

SymbolTableEntry*
parserUtil_handleFuncPrefix(char* name, unsigned int line);

void
parserUtil_handleFuncArgs();

unsigned
parserUtil_handleFuncbody();

SymbolTableEntry*
parserUtil_handleFuncdef(SymbolTableEntry* funcPrefix, unsigned totalLocals, unsigned int line);

char*
parserUtil_generateUnnamedFunctionName();

#endif