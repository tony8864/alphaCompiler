#include "dispatcher.h"

#include "../avm_types.h"
#include "../executors/arithmetic.h"
#include "../executors/assign.h"

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

typedef void (*execute_func_t)(instruction*);

unsigned char executionFinished = 0;
unsigned pc = 0;

#define execute_add execute_arithmetic
#define execute_sub execute_arithmetic
#define execute_mul execute_arithmetic
#define execute_div execute_arithmetic
#define execute_mod execute_arithmetic

execute_func_t executeFuncs[] = {
    execute_assign,
    execute_add,
    execute_sub,
    execute_mul,
    execute_div,
    execute_mod
};

void
execute_cycle() {
    if (executionFinished) {
        return;
    }
    else if (pc == AVM_ENDING_PC) {
        executionFinished = 1;
        return;
    }
    else {
        assert(pc < AVM_ENDING_PC);
        instruction* instr = code + pc;
        assert(instr->opcode >= 0 && instr->opcode <= AVM_MAX_INSTRUCTIONS);
        unsigned oldPc = pc;
        (*executeFuncs[instr->opcode])(instr);
        if (pc == oldPc) {
            ++pc;
        }
    }
}

unsigned char
isExecutionFinished() {
    return executionFinished;
}