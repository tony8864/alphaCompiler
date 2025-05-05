#include "dispatcher.h"
#include "../loader/loader.h"

#include <stdlib.h>
#include <assert.h>
#include <string.h>

static double* numConsts = NULL;
static char** stringConsts = NULL;
static char** namedLibFuncs = NULL;
static userfunc* userFuncs = NULL;
instruction* code = NULL;
static unsigned codeSize;
static unsigned int totalGlobals;

unsigned char executionFinished = 0;
unsigned pc = 0;
unsigned currLine = 0;

// execution cycle
#define AVM_ENDING_PC (loader_getCodeSize())
#define AVM_MAX_INSTRUCTIONS (unsigned) nop_v

// stack
#define AVM_STACKENV_SIE 4
#define AVM_STACKSIZE 4096
#define N AVM_STACKSIZE
#define AVM_WIPEOUT(m) memset(&(m), 0, sizeof(m))

avm_memcell stack[AVM_STACKSIZE];
avm_memcell ax, bx, cx;
avm_memcell retval;
unsigned top, topsp;

// arithmetic executors
#define execute_add execute_arithmetic
#define execute_sub execute_arithmetic
#define execute_mul execute_arithmetic
#define execute_div execute_arithmetic
#define execute_mod execute_arithmetic

typedef double (*arithmetic_func_t)(double x, double y);
typedef void (*execute_func_t)(instruction*);
typedef void (*memclear_func_t)(avm_memcell*);

/* -------------------------------------------------- Static Declarations  -------------------------------------------------- */
static void
initstack();

static void
memcell_clear(avm_memcell* m);

static void
memclear_string(avm_memcell* m);

static void
memclear_table(avm_memcell* m);

static void
initNumConsts();

static void
initStrConsts();

static void
initNamedLibs();

static void
initUserFuncs();

static void
initCodeSize();

static void
initInstructions();

static avm_memcell*
translate_operand(vmarg* arg, avm_memcell* reg);

static void
assign(avm_memcell* lv, avm_memcell* rv);

static void
execute_assign(instruction* instr);

static double
add_impl(double x, double y);

static double
sub_impl(double x, double y);

static double
mul_impl(double x, double y);

static double
div_impl(double x, double y);

static double
mod_impl(double x, double y);

static void
execute_arithmetic(instruction* instr);

arithmetic_func_t arithmeticFuncs[] = {
    add_impl,
    sub_impl,
    mul_impl,
    div_impl,
    mod_impl
};

execute_func_t executeFuncs[] = {
    execute_assign,
    execute_add,
    execute_sub,
    execute_mul,
    execute_div,
    execute_mod,
};

memclear_func_t memclearFuncs[] = {
    0,
    memclear_string,
    0,
    memclear_table,
    0,
    0,
    0,
    0
};

/* -------------------------------------------------- Implementation  -------------------------------------------------- */
void
dispatcher_initialize(FILE* binaryFile) {
    loader_loadBinaryFile(binaryFile);
    loader_loadCode();

    initstack();
    initNumConsts();
    initStrConsts();
    initNamedLibs();
    initUserFuncs();
    initCodeSize();
    initInstructions();
}

static void
memcell_clear(avm_memcell* m) {
    if (m->type != undef_m) {
        memclear_func_t f = memclearFuncs[m->type];
        if (f) {
            (*f)(m);
        }
        m->type = undef_m;
    }
}

static void
memclear_string(avm_memcell* m) {
    assert(m->data.strval);
    free(m->data.strval);
}

static void
memclear_table(avm_memcell* m) {

}

void
dispatcher_execute_cycle() {
    while (!executionFinished && pc != AVM_ENDING_PC) {
        instruction* instr = code + pc;

        assert(
            instr->opcode >= 0 &&
            instr->opcode <= AVM_MAX_INSTRUCTIONS
        );

        unsigned oldPc = pc;
        (*executeFuncs[instr->opcode])(instr);
        if (pc == oldPc) { pc++; }
    }
}


/* -------------------------------------------------- Static Definitions  -------------------------------------------------- */
static void
initstack() {
    for (unsigned i = 0; i < AVM_STACKSIZE; i++) {
        AVM_WIPEOUT(stack[i]);
        stack[i].type = undef_m;
    }
}

static void
initNumConsts() {
    numConsts = loader_getNumConsts();
}

static void
initStrConsts() {
    stringConsts = loader_getStringConsts();
}

static void
initNamedLibs() {
    namedLibFuncs = loader_getLibFuncs();
}

static void
initUserFuncs() {
    userFuncs = loader_getUserFuncs();
}

static void
initCodeSize() {
    codeSize = loader_getCodeSize();
}

static void
initInstructions() {
    code = loader_getInstructions();
}

static avm_memcell*
translate_operand(vmarg* arg, avm_memcell* reg) {
    switch (arg->type) {
        case global_a: {
            return &stack[AVM_STACKSIZE - 1 - arg->val];
        }
        case local_a: 
        case formal_a:
        case retval_a:
        case number_a: {
            reg->type = number_m;
            reg->data.numVal = numConsts[arg->val];
            return reg;
        }
        case string_a:
        case userfunc_a:
        case libfunc_a:
        case bool_a:
        case nil_a: break;
    }
}

static void
execute_assign(instruction* instr) {
    avm_memcell* lv = translate_operand(&instr->result, NULL);
    avm_memcell* rv = translate_operand(&instr->arg1, &ax);

    assert(&stack[top] < lv && lv <= &stack[N - 1] || lv == &retval);
    assert(rv);

    assign(lv, rv);
}

static void
assign(avm_memcell* lv, avm_memcell* rv) {
    if (lv == rv) {
        return;
    }

    // table checks

    if (rv->type == undef_m) {
        printf("Assigning from 'undef' contnet!\n");
    }

    memcell_clear(lv);

    memcpy(lv, rv, sizeof(avm_memcell));

    if (lv->type == string_m) {
        lv->data.strval = strdup(rv->data.strval);
    }
}

static void
execute_arithmetic(instruction* instr) {
    avm_memcell* lv = translate_operand(&instr->result, NULL);
    avm_memcell* rv1 = translate_operand(&instr->arg1, &ax);
    avm_memcell* rv2 = translate_operand(&instr->arg2, &bx);

    assert(&stack[top] < lv && lv <= &stack[N - 1] || lv == &retval);
    assert(rv1 && rv2);

    if (rv1->type != number_m || rv2->type != number_m) {
        printf("Error: Not a number in arithmetic.\n");
        exit(1);
    }

    arithmetic_func_t op = arithmeticFuncs[instr->opcode - add_v];
    memcell_clear(lv);
    lv->type = number_m;
    lv->data.numVal = (*op)(rv1->data.numVal, rv2->data.numVal);

    printf("lv value: %d\n", (int)lv->data.numVal);
}

static double
add_impl(double x, double y) { return x + y; }

static double
sub_impl(double x, double y) { return x - y; }

static double
mul_impl(double x, double y) { return x * y; }

static double
div_impl(double x, double y) { return x / y; }

static double
mod_impl(double x, double y) { return (unsigned) x % (unsigned) y; }