#ifndef AVM_TYPES
#define AVM_TYPES

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

typedef struct avm_table {

} avm_table;

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

typedef struct avm_memcell {
    avm_memcell_t type;
    union {
        double numVal;
        char* strVal;
        unsigned char boolVal;
        avm_table* tableVal;
        unsigned funcVal;
        char* libfuncVal;
    } data;
} avm_memcell;

// avm
extern unsigned         codeSize;
extern instruction*     code;

extern avm_memcell* avm_translate_operand(vmarg* arg, avm_memcell* reg);

extern void avm_warning(char* str);

// memory
#define AVM_STACKENV_SIZE   4
#define AVM_STACKSIZE       4094
#define N                   AVM_STACKSIZE

extern avm_memcell stack[AVM_STACKSIZE];
extern avm_memcell   ax, bx, cx;
extern avm_memcell   retval;
extern unsigned top, topsp;

extern void avm_memcellclear(avm_memcell* m);

// dispatcher
#define AVM_ENDING_PC           codeSize
#define AVM_MAX_INSTRUCTIONS    (unsigned) nop_v

extern void registerlibfuncs();

extern unsigned char    executionFinished;
extern unsigned         pc;

// loader
extern double       consts_getnumber    (unsigned i);
extern char*        consts_getstring    (unsigned i);
extern char*        libfuncs_getused    (unsigned i);
extern userfunc*    userfucns_getfunc   (unsigned i);

#endif