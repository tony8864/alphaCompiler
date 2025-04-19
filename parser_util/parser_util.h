#ifndef PARSER_UTIL_H
#define PARSER_UTIL_H

#include "../icode/icode.h"
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

Expr*
parserUtil_handleLocalIdentifier(const char* name, unsigned int line);

SymbolTableEntry*
parserUtil_handleFormalArgument(const char* name, unsigned int line);

Expr*
parserUtil_handleIdentifier(const char* name, unsigned int line);

Expr*
parserUtil_habdleGlobalLookup(const char* name, unsigned int line);

SymbolTableEntry*
parserUtil_handleFuncPrefix(char* name, unsigned int line);

void
parserUtil_handleFuncArgs();

unsigned
parserUtil_handleFuncbody();

SymbolTableEntry*
parserUtil_handleFuncdef(SymbolTableEntry* funcPrefix, unsigned totalLocals, unsigned int line);

Expr*
parserUtil_handleLvalueIdentifierTableItem(Expr* lv, char* identifier, unsigned int line);

Expr*
parserUtil_handleLvalueExprTableItem(Expr* lv, Expr* e, unsigned int line);

Expr*
parserUtil_handlePrimary(Expr* lv, unsigned int line);

char*
parserUtil_generateUnnamedFunctionName();

Expr*
parserUtil_handleAssignExpr(Expr* lv, Expr* e, unsigned int line);

Expr*
parserUtil_newConstnumExpr(double i);

#endif