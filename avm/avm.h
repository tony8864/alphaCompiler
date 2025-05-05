#ifndef AVM_H
#define AVM_H

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

typedef struct vmarg {
    vmarg_t type;
    unsigned val;
} vmarg;

typedef struct instruction {
    vmopcode opcode;
    vmarg result;
    vmarg arg1;
    vmarg arg2;
    unsigned srcLine;
} instruction;

typedef struct userfunc {
    unsigned address;
    unsigned localSize;
    char* id;
} userfunc;

typedef enum {
    number_m,
    string_m,
    bool_m,
    table_m,
    userfunc_m,
    libfunc_m,
    nil_m,
    undef_m
} avm_memcell_t;

typedef struct avm_table {

} avm_table;

typedef struct avm_memcell {
    avm_memcell_t type;
    union {
        double numVal;
        char* strval;
        unsigned char boolVal;
        avm_table* tableVal;
        char* libfuncVal;
    } data;
} avm_memcell;

#endif