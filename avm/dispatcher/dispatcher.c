#include "dispatcher.h"

#include "../avm_types.h"
#include "../executors/arithmetic.h"
#include "../executors/relational.h"
#include "../executors/assign.h"
#include "../executors/equal.h"

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

static void
execute_nop(instruction* instr);

static void
execute_jump(instruction* instr);

typedef void (*execute_func_t)(instruction*);

unsigned char executionFinished = 0;
unsigned pc = 0;

#define execute_add execute_arithmetic
#define execute_sub execute_arithmetic
#define execute_mul execute_arithmetic
#define execute_div execute_arithmetic
#define execute_mod execute_arithmetic

#define execute_jge execute_relational
#define execute_jgt execute_relational
#define execute_jle execute_relational
#define execute_jlt execute_relational

execute_func_t executeFuncs[] = {
    execute_assign,
    execute_add,
    execute_sub,
    execute_mul,
    execute_div,
    execute_mod,
    execute_nop, // uminus
    execute_nop, // and
    execute_nop, // or
    execute_nop, // not
    execute_jump,
    execute_jeq,
    execute_jne,
    execute_jle,
    execute_jge,
    execute_jlt,
    execute_jgt
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

static void
execute_nop(instruction* instr) {
    return;
}

static void
execute_jump(instruction* instr) {
    assert(instr->result.type == label_a);
    pc = instr->result.val;
}