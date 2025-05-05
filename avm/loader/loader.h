#ifndef LOADER_H
#define LOADER_H

#include "../avm.h"
#include <stdio.h>

void
loader_loadBinaryFile(FILE* file);

void
loader_loadCode();

double*
loader_getNumConsts();

char**
loader_getStringConsts();

char**
loader_getLibFuncs();

userfunc*
loader_getUserFuncs();

instruction*
loader_getInstructions();

unsigned
loader_getCodeSize();

#endif