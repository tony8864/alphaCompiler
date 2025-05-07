#include "equal.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

typedef unsigned char (*tobool_func_t)(avm_memcell*);
typedef unsigned char (*equal_func_t)(avm_memcell* m1, avm_memcell* m2);

unsigned char number_tobool(avm_memcell* m)     { return m->data.numVal != 0; }
unsigned char string_tobool(avm_memcell* m)     { return m->data.strVal[0] != 0; }
unsigned char bool_tobool(avm_memcell* m)       { return m->data.boolVal; }
unsigned char userfunc_tobool(avm_memcell* m)   { return 1; }
unsigned char libfunc_tobool(avm_memcell* m)    { return 1; }
unsigned char nil_tobool(avm_memcell* m)        { return 0; }
unsigned char undef_tobool(avm_memcell* m)      { return 0; }

tobool_func_t toboolFuncs[] = {
    number_tobool,
    string_tobool,
    bool_tobool,
    userfunc_tobool,
    libfunc_tobool,
    nil_tobool,
    undef_tobool
};

unsigned char number_equal(avm_memcell* m1, avm_memcell* m2) {
    return m1->data.numVal == m2->data.numVal;
}

unsigned char string_equal(avm_memcell* m1, avm_memcell* m2) {
    return strcmp(m1->data.strVal, m2->data.strVal) == 0;
}

unsigned char bool_equal(avm_memcell* m1, avm_memcell* m2) {
    return m1->data.boolVal == m2->data.boolVal;
}

unsigned char table_equal(avm_memcell* m1, avm_memcell* m2) {
    return m1->data.tableVal == m2->data.tableVal;
}

unsigned char userfunc_equal(avm_memcell* m1, avm_memcell* m2) {
    return m1->data.funcVal == m2->data.funcVal;
}

unsigned char libfunc_equal(avm_memcell* m1, avm_memcell* m2) {
    return strcmp(m1->data.libfuncVal, m2->data.libfuncVal) == 0;
}

equal_func_t equalFuncs[] = {
    number_equal,
    string_equal,
    bool_equal,
    table_equal,
    userfunc_equal,
    libfunc_equal,
    NULL,
    NULL
};

unsigned char avm_tobool(avm_memcell* m) {
    assert(m->type >= 0 && m->type < undef_m);
    return (*toboolFuncs[m->type])(m);
}

void
execute_jeq(instruction* instr) {
    assert(instr->result.type == label_a);

    avm_memcell* rv1 = avm_translate_operand(&instr->arg1, &ax);
    avm_memcell* rv2 = avm_translate_operand(&instr->arg2, &bx);

    unsigned char result = 0;

    if (rv1->type == undef_m || rv2->type == undef_m) {
        printf("Undef involved in equality!");
        exit(1);
    }

    if (rv1->type == bool_m || rv2->type == bool_m) {
        result = (avm_tobool(rv1) == avm_tobool(rv2));
    }
    else if (rv1->type == nil_m || rv2->type == nil_m) {
        result = rv1->type == nil_m && rv2->type == nil_m;
    }
    else if (rv1->type != rv2->type) {
        printf("Mismatch in equality.\n");
        exit(1);
    }
    else {
      equal_func_t eqFunc = equalFuncs[rv1->type];
      result = (*eqFunc)(rv1, rv2); 
    }

    if (result) {
        pc = instr->result.val;
    }
}

void
execute_jne(instruction* instr) {
    assert(instr->result.type == label_a);

    avm_memcell* rv1 = avm_translate_operand(&instr->arg1, &ax);
    avm_memcell* rv2 = avm_translate_operand(&instr->arg2, &bx);

    unsigned char result = 0;

    if (rv1->type == undef_m || rv2->type == undef_m) {
        printf("Undef involved in equality!");
        exit(1);
    }

    if (rv1->type == bool_m || rv2->type == bool_m) {
        result = (avm_tobool(rv1) == avm_tobool(rv2));
    }
    else if (rv1->type == nil_m || rv2->type == nil_m) {
        result = rv1->type == nil_m && rv2->type == nil_m;
    }
    else if (rv1->type != rv2->type) {
        printf("Mismatch in equality.\n");
        exit(1);
    }
    else {
      equal_func_t eqFunc = equalFuncs[rv1->type];
      result = (*eqFunc)(rv1, rv2); 
    }

    if (!result) {
        pc = instr->result.val;
    }
}