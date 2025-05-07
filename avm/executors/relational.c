#include "relational.h"

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

typedef unsigned char (*relational_func_t)(double x, double y);

unsigned char jge_impl(double x, double y) { return x >= y; }
unsigned char jle_impl(double x, double y) { return x <= y; }
unsigned char jgt_impl(double x, double y) { return x > y; }
unsigned char jlt_impl(double x, double y) { return x < y; }

relational_func_t relationalFuncs[] = {
    jle_impl,
    jge_impl,
    jlt_impl,
    jgt_impl
};

void
execute_relational(instruction* instr) {

    assert(instr->result.type == label_a);

    avm_memcell* rv1 = avm_translate_operand(&instr->arg1, &ax);
    avm_memcell* rv2 = avm_translate_operand(&instr->arg2, &bx);

    if (rv1->type != number_m || rv2->type != number_m) {
        printf("AVM Error: Not a number in relational.\n");
        exit(1);
    }

    unsigned char result;
    relational_func_t op;

    op = relationalFuncs[instr->opcode - jle_v];
    result = (*op)(rv1->data.numVal, rv2->data.numVal);

    if (result) {
        pc = instr->result.val;
    }
}