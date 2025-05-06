#ifndef ASSIGN_H
#define ASSIGN_H

#include "../avm_types.h"

void
execute_assign(instruction* instr);

void
avm_assign(avm_memcell* lv, avm_memcell* rv);

#endif