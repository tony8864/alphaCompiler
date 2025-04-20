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

Expr*
parserUtil_handlePrimaryFuncdef(SymbolTableEntry* entry);

char*
parserUtil_generateUnnamedFunctionName();

Expr*
parserUtil_handleAssignExpr(Expr* lv, Expr* e, unsigned int line);

Expr*
parserUtil_newConstnumExpr(double i);

Expr*
parserUtil_newBoolExpr(unsigned char bool);

Call*
parserUtil_handleMethodCall(char* identifier, Expr* elist);

Expr*
parserUtil_handleCall(Expr* call, Expr* elist, unsigned int line);

Expr*
parserUtil_handleCallSuffix(Expr* lv, Call* callsuffix, unsigned int line);

Call*
parserUtil_handleNormCall(Expr* elist);

Expr*
parserUtil_handleCallFuncdef(SymbolTableEntry* funcdef, Expr* elist, unsigned int line);

Expr*
parserUtil_handleElist(Expr* elist, Expr* e);

Expr*
parserUtil_handleUminusExpr(Expr* e, unsigned int line);

Expr*
parserUtil_handleNotExpr(Expr* e, unsigned int line);

Expr*
parserUtil_handleLvalueIncrement(Expr* lv, unsigned int line);

Expr*
parserUtil_handleIncrementLvalue(Expr* lv, unsigned int line);

Expr*
parserUtil_handleLvalueDecrement(Expr* lv, unsigned int line);

Expr*
parserUtil_handleDecrementLvalue(Expr* lv, unsigned int line);

Expr*
parserUtil_handleMakeElistTable(Expr* elist, unsigned int line);

Indexed*
parserUtil_newIndexed(Expr* key, Expr* value);

Indexed*
parserUtil_handleIndexed(Indexed* indexedList, Indexed* indexed);

Expr*
parserUtil_handleMakeIndexedTable(Indexed* indexedList, unsigned int line);

Expr*
parserUtil_handleArithmeticExpr(Expr* expr1, Expr* expr2, IOPCodeType op, unsigned int line);

Expr*
parserUtil_handleRelationalExpr(Expr* expr1, Expr* expr2, IOPCodeType op, unsigned int line);

Expr*
parserUtil_handleBooleanExpr(Expr* expr1, Expr* expr2, IOPCodeType op, unsigned int line);

#endif