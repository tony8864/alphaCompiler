#include "function.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

static unsigned totalActuals = 0;

/* ------------------------------------------- Static Declarations ------------------------------------------- */
static void
avm_calllibfunc(char* id);

static void
avm_dec_top();

static void
avm_push_envvalue(unsigned val);

static unsigned
avm_get_envnvalue(unsigned i);

static void
avm_callsaveenvironment();

static void
libfunc_print();

static void
libfunc_input();

static void
libfunc_objectmemberkeys();

static void
libfunc_objecttotalmembers();

static void
libfunc_objectcopy();

static void
libfunc_totalarguments();

static void
libfunc_argument();

static void
libfunc_typeof();

static void
libfunc_strtonum();

static void
libfunc_sqrt();

static void
libfunc_cos();

static void
libfunc_sin();

typedef void (*library_func_t)(void);

typedef struct {
    char* name;
    library_func_t func;
} libfunc_entry;

libfunc_entry libfuncMap[] = {
    { "print",              libfunc_print },
    { "input",              libfunc_input },
    { "objectmemberkeys",   libfunc_objectmemberkeys },
    { "objecttotalmembers", libfunc_objecttotalmembers },
    { "objectcopy",         libfunc_objectcopy },
    { "totalarguments",     libfunc_totalarguments },
    { "argument",           libfunc_argument },
    { "typeof",             libfunc_typeof },
    { "strtonum",           libfunc_strtonum },
    { "sqrt",               libfunc_sqrt },
    { "cos",                libfunc_cos },
    { "sin",                libfunc_sin },
};

#define LIBFUNC_COUNT (sizeof(libfuncMap) / sizeof(libfuncMap[0]))

static library_func_t
get_libfunc(char* name);

/* ------------------------------------------- Implementation ------------------------------------------- */
void
execute_call(instruction* instr) {
    avm_memcell* func = avm_translate_operand(&instr->arg1, &ax);
    assert(func);
    printf("func type: %u pc: %u\n", func->type, pc);
    switch (func->type) {
        case userfunc_m: {
            avm_callsaveenvironment();
            pc = func->data.funcVal;
            assert(code[pc].opcode == funcenter_v);
            printf("calling user func.\n");
            break;
        }
        case string_m: {
            avm_calllibfunc(func->data.strVal);
            break;
        }
        case libfunc_m: {
            avm_calllibfunc(func->data.strVal);
            break;
        }
        default: {
            printf("Cannot bind to function.\n");
            exit(1);
        }
    }
}

void
execute_pusharg(instruction* instr) {

}

void
execute_funcenter(instruction* instr) {

}

void
execute_funcexit(instruction* instr) {
    
}

/* ------------------------------------------- Extern Definition ------------------------------------------- */

/* ------------------------------------------- Static Definitions ------------------------------------------- */
static library_func_t
get_libfunc(char* name) {
    for (int i = 0; i < LIBFUNC_COUNT; i++) {
        if (strcmp(libfuncMap[i].name, name) == 0) {
            return libfuncMap[i].func;
        }
    }
    return NULL;
}

static void avm_calllibfunc(char* id) {

}

static void
avm_dec_top() {
    if (!top) { // stack overflow
        printf("Stack overflow.\n");
        exit(1);
    }
    else {
        top--;
    }
}

static void
avm_push_envvalue(unsigned val) {
    stack[top].type = number_m;
    stack[top].data.numVal = val;
    avm_dec_top();
}

static unsigned
avm_get_envnvalue(unsigned i) {
    assert(stack[i].type == number_m);
    unsigned val = (unsigned) stack[i].data.numVal;
    assert(stack[i].data.numVal == ((double) val));
    return val;
}

static void
avm_callsaveenvironment() {
    avm_push_envvalue(totalActuals);
    assert(code[pc].opcode == call_v);
    avm_push_envvalue(pc + 1);
    avm_push_envvalue(top + totalActuals + 2);
    avm_push_envvalue(topsp);
}

static void
libfunc_print() {

}

static void
libfunc_input() {

}

static void
libfunc_objectmemberkeys() {

}

static void
libfunc_objecttotalmembers() {

}

static void
libfunc_objectcopy() {

}

static void
libfunc_totalarguments() {

}

static void
libfunc_argument() {

}

static void
libfunc_typeof() {

}

static void
libfunc_strtonum() {

}

static void
libfunc_sqrt() {

}

static void
libfunc_cos() {

}

static void
libfunc_sin() {

}