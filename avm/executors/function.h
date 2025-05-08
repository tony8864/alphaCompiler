#ifndef FUNCTION_H
#define FUNCTION_H

#include "../avm_types.h"

void
execute_call(instruction* instr);

void
execute_pusharg(instruction* instr);

void
execute_funcenter(instruction* instr);

void
execute_funcexit(instruction* instr);

#endif