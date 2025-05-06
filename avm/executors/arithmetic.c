#include "arithmetic.h"

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

typedef double (*arithmetic_func_t)(double x, double y);

static double add_impl(double x, double y) { return x + y; }
static double sub_impl(double x, double y) { return x - y; }
static double mul_impl(double x, double y) { return x * y; }
static double div_impl(double x, double y) { return x / y; }
static double mod_impl(double x, double y) { return (unsigned )x % (unsigned) y; }

arithmetic_func_t arithmeticFuncs[] = {
    add_impl,
    sub_impl,
    mul_impl,
    div_impl,
    mod_impl
};

void
execute_arithmetic(instruction* instr) {
    avm_memcell* lv = avm_translate_operand(&instr->result, NULL);
    avm_memcell* rv1 = avm_translate_operand(&instr->arg1, &ax);
    avm_memcell* rv2 = avm_translate_operand(&instr->arg2, &bx);

    assert(lv && (&stack[N - 1] >= lv && lv > &stack[top]) || lv == &retval);
    assert(rv1 && rv2);

    if (rv1->type != number_m || rv2->type != number_m) {
        printf("AVM Error: Not a number in arithmetic.\n");
        exit(1);
    }

    arithmetic_func_t op = arithmeticFuncs[instr->opcode - add_v];
    avm_memcellclear(lv);
    lv->type = number_m;
    lv->data.numVal = (*op)(rv1->data.numVal, rv2->data.numVal);
}