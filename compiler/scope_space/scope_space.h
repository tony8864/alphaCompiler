#ifndef SCOPE_SPACE_H
#define SCOPE_SPACE_H

typedef enum { PROGRAMVAR, FUNCTIONLOCAL, FORMALARG } ScopeSpaceType;

void
scopeSpace_initialize();

void
scopeSpace_cleanup();

ScopeSpaceType
scopeSpace_currentScope();

unsigned
scopeSpace_currentScopeOffset();

void
scopeSpace_incrementCurrentScopeOffset();

void
scopeSpace_pushCurrentOffset();

void
scopeSpace_enterScopeSpace();

void
scopeSpace_resetFormatArgsOffset();

void
scopeSpace_resetLocalOffset();

void
scopeSpace_exitScopeSpace();

void
scopeSpace_restoreLocalOffset();

void
scopeSpace_exitFuncBlock();

void
scopeSpace_enterFuncFormalScope();

#endif