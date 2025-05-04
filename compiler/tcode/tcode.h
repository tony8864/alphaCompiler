#ifndef TCODE_H
#define TCODE_H

#include "../icode/icode.h"
#include "../quad/quad.h"

typedef enum {
    assign_v,       add_v,          sub_v,
    mul_v,          div_v,          mod_v,
    uminus_v,       and_v,          or_v,
    not_v,          jump_v,         jeq_v,      jne_v,
    jle_v,          jge_v,          jlt_v,
    jgt_v,          call_v,         pusharg_v,
    funcenter_v,    funcexit_v,     newtable_v,
    tablegetelem_v, tablesetelem_v, nop_v,
} vmopcode;

typedef enum {
    label_a,
    global_a,
    formal_a,
    local_a,
    number_a,
    string_a,
    bool_a,
    nil_a,
    userfunc_a,
    libfunc_a,
    retval_a,
    notype_a,
} vmarg_t;

typedef struct vmarg vmarg;
typedef struct instruction instruction;
typedef struct userfunc userfunc;

void
tcode_initialize();

unsigned int
tcode_nextInstructionLabel();

void
tcode_printInstructions();

void
tcode_generateInstructions();

void
tcode_printUserFuncs();

void
tcode_printNamedLibs();

void
tcode_printStringConsts();

void
tcode_printNumConsts();

void
tcode_createBinaryFile(char* filename);

#endif