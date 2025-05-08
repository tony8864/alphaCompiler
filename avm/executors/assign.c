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

void
avm_assign(avm_memcell* lv, avm_memcell* rv) {
    if (lv == rv) {
        return;
    }

    // table checks

    if (rv->type == undef_m) {
        avm_warning("Assigning from undef content!");
    }

    avm_memcellclear(lv);

    memcpy(lv, rv, sizeof(avm_memcell));

    if (lv->type == string_m) {
        lv->data.strVal = strdup(rv->data.strVal);
    }
    else if (lv->type == table_m) {
        // increment ref counter
    }
}