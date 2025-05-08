#include "assign.h"

#include <stdio.h>
#include <assert.h>
#include <string.h>

void
execute_assign(instruction* instr) {
    avm_memcell* lv = avm_translate_operand(&instr->result, NULL);
    avm_memcell* rv = avm_translate_operand(&instr->arg1, &ax);
    
    assert(lv && (&stack[N - 1] >= lv && lv > &stack[top] || lv == &retval));
    assert(rv);

    avm_assign(lv, rv);   
}