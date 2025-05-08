#include "../avm_types.h"
#include "loader.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

typedef struct avm_constants {
    double* numConsts;
    char** stringConsts;
    char** namedLibFuncs;
    instruction* instructions;
    userfunc* userFuncs;

    unsigned totalNumConsts;
    unsigned totalStringConsts;
    unsigned totalNamedLibFuncs;
    unsigned totalUserFuncs;
    unsigned totalInstructions;

    unsigned totalGlobals;
} avm_constants;

static FILE* binaryFile = NULL;

/* ------------------------------------------ Static Declarations ------------------------------------------ */
static void
open_binaryFile(char* filename);

static void
read_magicNumber();

static void
read_strings(avm_constants* consts);

static void
print_strings(avm_constants* consts);

static void
read_nums(avm_constants* consts);

static void
print_nums(avm_constants* consts);

static void
read_userfuncs(avm_constants* consts);

static void
print_userfuncs(avm_constants* consts);

static void
read_libfuncs(avm_constants* consts);

static void
print_libfuncs(avm_constants* consts);

static void
read_instructions(avm_constants* consts);

static void
print_instructions(avm_constants* consts);

static void
read_totalGlobals(avm_constants* consts);

static void
print_totalGlobals(avm_constants* consts);

static void
read_vmarg(vmarg* arg);

static char*
vmarg_to_string(vmarg arg);

static const char*
vmopcode_to_string(vmopcode op);

static const char*
vmarg_type_to_string(vmarg_t type);

/* ------------------------------------------ Implementation ------------------------------------------ */
avm_constants*
loader_load_avm_constants(char* filename) {

    open_binaryFile(filename);

    read_magicNumber();

    avm_constants* consts;

    consts = malloc(sizeof(avm_constants));
    if (!consts) {
        printf("Error allocating memory for avm consts.\n");
        exit(1);
    }

    read_strings(consts);
    read_nums(consts);
    read_userfuncs(consts);
    read_libfuncs(consts);
    read_instructions(consts);
    read_totalGlobals(consts);

    print_strings(consts);
    print_nums(consts);
    print_userfuncs(consts);
    print_libfuncs(consts);
    print_instructions(consts);
    print_totalGlobals(consts);

    return consts;
}

double
loader_consts_getnumber(avm_constants* consts, unsigned index) {
    assert(consts && consts->numConsts && (index < consts->totalNumConsts));
    return consts->numConsts[index];
}

userfunc
loader_consts_getuserfunc(avm_constants* consts, unsigned index) {
    assert(consts && consts->userFuncs && (index < consts->totalUserFuncs));
    return consts->userFuncs[index];
}

char*
loader_consts_getstring(avm_constants* consts, unsigned index) {
    assert(consts && consts->stringConsts && (index < consts->totalStringConsts));
    return consts->stringConsts[index];
}

unsigned
loader_getTotalGlobals(avm_constants* consts) {
    assert(consts);
    return consts->totalGlobals;
}

instruction*
loader_getcode(avm_constants* consts) {
    assert(consts && consts->instructions);
    return consts->instructions;
}

unsigned
loader_getcodeSize(avm_constants* consts) {
    assert(consts && consts->totalInstructions);
    return consts->totalInstructions;
}

/* ------------------------------------------ Static Definitions ------------------------------------------ */
static void
open_binaryFile(char* filename) {
    binaryFile = fopen(filename, "r");
    if (!binaryFile) {
        printf("Error opening binary file.\n");
        exit(1);
    }
}

static void
read_magicNumber() {
    unsigned magicNumber;
    if (fscanf(binaryFile, "%u", &magicNumber) != 1) {
        printf("Error reading magic number.\n");
        exit(1);
    }
    
    if (magicNumber != 340200501) {
        printf("Magic number is incorrect.\n");
        exit(1);
    }
}

static void
read_strings(avm_constants* consts) {
    char** stringConsts;
    unsigned totalStrings;

    if (fscanf(binaryFile, "%u", &totalStrings) != 1) {
        printf("Error reading total strings from binary file.\n");
        exit(1);
    }

    stringConsts = malloc(sizeof(char*) * totalStrings);
    if (!stringConsts) {
        printf("Error allocating memory for string consts.\n");
        exit(1);
    }

    int len;
    int ch;

    for (int i = 0; i < totalStrings; i++) {
        if (fscanf(binaryFile, "%d", &len) != 1) {
            printf("Error reading length of string.\n");
            exit(1);
        }

        // consume space
        fgetc(binaryFile);

        stringConsts[i] = malloc(sizeof(char) * (len + 1));
        if (!stringConsts[i]) {
            printf("Error allocating memory for const string.\n");
            exit(1);
        }

        for (int j = 0; j < len; j++) {
            ch = fgetc(binaryFile);
            if (ch == EOF) {
                printf("Error: Unexpected EOF while reading string.\n");
                exit(1);
            }
            stringConsts[i][j] = (char) ch;
        }
        stringConsts[i][len] = '\0';
    }

    consts->stringConsts = stringConsts;
    consts->totalStringConsts = totalStrings;
}

static void
print_strings(avm_constants* consts) {
    for (int i = 0; i < consts->totalStringConsts; i++) {
        printf("%d: %s\n", i, consts->stringConsts[i]);
    }
}

static void
read_nums(avm_constants* consts) {
    unsigned totalNums;
    double* numConsts;

    if (fscanf(binaryFile, "%u", &totalNums) != 1) {
        printf("Error reading total nums.\n");
        exit(1);
    }

    numConsts = malloc(sizeof(double) * totalNums);
    if (!numConsts) {
        printf("Error allocating memory for num consts.\n");
        exit(1);
    }

    double n;
    for (int i = 0; i < totalNums; i++) {
        if (fscanf(binaryFile, "%lf", &n) != 1) {
            printf("Error reading num const.\n");
            exit(1);
        }
        numConsts[i] = n;
    }

    consts->totalNumConsts = totalNums;
    consts->numConsts = numConsts;
}

static void
print_nums(avm_constants* consts) {
    for (int i = 0; i < consts->totalNumConsts; i++) {
        printf("n_%d: %lf\n", i, consts->numConsts[i]);
    }
}

static void
read_userfuncs(avm_constants* consts) {
    unsigned totalUserfuncs;
    userfunc* userfuncs;

    if (fscanf(binaryFile, "%u", &totalUserfuncs) != 1) {
        printf("Error reading total user funcs from binary file.\n");
        exit(1);
    }

    userfuncs = malloc(sizeof(userfunc) * totalUserfuncs);
    if (!userfuncs) {
        printf("Error allocating memory for user funcs.\n");
        exit(1);
    }

    userfunc f;
    unsigned address, localsize;
    char* id;
    int len;
    char ch;

    for (int i = 0; i < totalUserfuncs; i++) {
        if (fscanf(binaryFile, "%u", &address) != 1) {
            printf("Error reading address of user function.\n");
            exit(1);
        }

        if (fscanf(binaryFile, "%u", &localsize) != 1) {
            printf("Error reading localsize of user function.\n");
            exit(1);
        }

        if (fscanf(binaryFile, "%d", &len) != 1) {
            printf("Error reading len of user function.\n");
            exit(1);
        }

        // consume space
        fgetc(binaryFile);

        id = malloc(sizeof(char) * (len + 1));
        if (!id) {
            printf("Error allocating memory for user func id.\n");
            exit(1);
        }

        for (int j = 0; j < len; j++) {
            ch = fgetc(binaryFile);
            if (ch == EOF) {
                printf("Error: Unexpected EOF while reading user func.\n");
                exit(1);
            }
            id[j] = (char) ch;
        }
        id[len] = '\0';

        userfuncs[i].address = address;
        userfuncs[i].localSize = localsize;
        userfuncs[i].id = id;
    }

    consts->totalUserFuncs = totalUserfuncs;
    consts->userFuncs = userfuncs;
}

static void
print_userfuncs(avm_constants* consts) {
    for (int i = 0; i < consts->totalUserFuncs; i++) {
        userfunc f = consts->userFuncs[i];
        printf("addr: %u locals: %u id: %s\n", f.address, f.localSize, f.id);
    }
}

static void
read_libfuncs(avm_constants* consts) {
    char** libfuncs;
    unsigned totalLibs;

    if (fscanf(binaryFile, "%u", &totalLibs) != 1) {
        printf("Error reading total libs from binary file.\n");
        exit(1);
    }

    libfuncs = malloc(sizeof(char*) * totalLibs);
    if (!libfuncs) {
        printf("Error allocating memory for lib consts.\n");
        exit(1);
    }

    int len;
    int ch;

    for (int i = 0; i < totalLibs; i++) {
        if (fscanf(binaryFile, "%d", &len) != 1) {
            printf("Error reading length of lib name.\n");
            exit(1);
        }

        // consume space
        fgetc(binaryFile);

        libfuncs[i] = malloc(sizeof(char) * (len + 1));
        if (!libfuncs[i]) {
            printf("Error allocating memory for const lib.\n");
            exit(1);
        }

        for (int j = 0; j < len; j++) {
            ch = fgetc(binaryFile);
            if (ch == EOF) {
                printf("Error: Unexpected EOF while reading lib name.\n");
                exit(1);
            }
            libfuncs[i][j] = (char) ch;
        }
        libfuncs[i][len] = '\0';
    }

    consts->namedLibFuncs = libfuncs;
    consts->totalNamedLibFuncs = totalLibs;
}

static void
print_libfuncs(avm_constants* consts) {
    for (int i = 0; i < consts->totalNamedLibFuncs; i++) {
        printf("%d %s\n", i, consts->namedLibFuncs[i]);
    }
}

static void
read_instructions(avm_constants* consts) {
    unsigned codesize;
    instruction* code;
    
    if (fscanf(binaryFile, "%u", &codesize) != 1) {
        printf("Error reading code size from binary file.\n");
        exit(1);
    }

    code = malloc(sizeof(instruction) * codesize);
    if (!code) {
        printf("Error allocating memory for code.\n");
        exit(1);
    }

    vmarg result, arg1, arg2;
    vmopcode opcode;

    for (int i = 0; i < codesize; i++) {
        if (fscanf(binaryFile, "%u", &opcode) != 1) {
            printf("Error reading opcode.\n");
            exit(1);
        }

        read_vmarg(&result);
        read_vmarg(&arg1);
        read_vmarg(&arg2);

        code[i].opcode = opcode;
        code[i].result = result;
        code[i].arg1 = arg1;
        code[i].arg2 = arg2;
    }

    consts->totalInstructions = codesize;
    consts->instructions = code;
}

static void
print_instructions(avm_constants* consts) {
    printf("---------------------------------------------INSTRUCTIONS---------------------------------------------\n");
    printf("%-10s %-20s %-20s %-20s %-20s %-10s\n",
         "Instr.", "opcode", "result", "arg1", "arg2", "line");
    printf("---------------------------------------------------------------------------------------------------------\n");

    for (int i = 0; i < consts->totalInstructions; i++) {
        instruction instr = consts->instructions[i];

        printf("%-10d %-20s %-20s %-20s %-20s %-10d\n",
            i,
            vmopcode_to_string(instr.opcode),
            vmarg_to_string(instr.result),
            vmarg_to_string(instr.arg1),
            vmarg_to_string(instr.arg2),
            instr.srcLine
        );
    }
    printf("\n");
}

static void
read_totalGlobals(avm_constants* consts) {
    unsigned totalGlobals;
    if (fscanf(binaryFile, "%u", &totalGlobals) != 1) {
        printf("Error reading total globals from binary file.\n");
        exit(1);
    }
    consts->totalGlobals = totalGlobals;
}

static void
print_totalGlobals(avm_constants* consts) {
    printf("Total globals: %u\n", consts->totalGlobals);
}

static void
read_vmarg(vmarg* arg) {
    vmarg_t type;
    unsigned value;

    if (fscanf(binaryFile, "%u", &type) != 1) {
        printf("Error reading vmarg type.\n");
        exit(1);
    }

    if (fscanf(binaryFile, "%u", &value) != 1) {
        printf("Error reading vmarg value.\n");
        exit(1);
    }

    arg->type = type;
    arg->val = value;
}

static char*
vmarg_to_string(vmarg arg) {

    if (arg.type == notype_a) {
        return "";
    }

    const char* type_str = vmarg_type_to_string(arg.type);

    char* result = malloc(64);
    if (!result) {
        fprintf(stderr, "Error: malloc failed in vmarg_to_string.\n");
        exit(1);
    }

    snprintf(result, 64, "[%s, %u]", type_str, arg.val);
    return result;
}

static const char*
vmopcode_to_string(vmopcode op) {
    switch (op) {
        case assign_v:       return "assign_v";
        case add_v:          return "add_v";
        case sub_v:          return "sub_v";
        case mul_v:          return "mul_v";
        case div_v:          return "div_v";
        case mod_v:          return "mod_v";
        case uminus_v:       return "uminus_v";
        case and_v:          return "and_v";
        case or_v:           return "or_v";
        case not_v:          return "not_v";
        case jump_v:         return "jump_v";
        case jeq_v:          return "jeq_v";
        case jne_v:          return "jne_v";
        case jle_v:          return "jle_v";
        case jge_v:          return "jge_v";
        case jlt_v:          return "jlt_v";
        case jgt_v:          return "jgt_v";
        case call_v:         return "call_v";
        case pusharg_v:      return "pusharg_v";
        case funcenter_v:    return "funcenter_v";
        case funcexit_v:     return "fucnexit_v";
        case newtable_v:     return "newtable_v";
        case tablegetelem_v: return "tablegetelem_v";
        case tablesetelem_v: return "tablesetelem_v";
        case nop_v:          return "nop_v";
        default:             return "UNKNOWN_OPCODE";
    }
}

static const char*
vmarg_type_to_string(vmarg_t type) {
    switch (type) {
        case label_a:      return "label";
        case global_a:     return "global";
        case formal_a:     return "formal";
        case local_a:      return "local";
        case number_a:     return "number";
        case string_a:     return "string";
        case bool_a:       return "bool";
        case nil_a:        return "nil";
        case userfunc_a:   return "userfunc";
        case libfunc_a:    return "libfunc";
        case retval_a:     return "retval";
        case notype_a:     return "";
        default:           return "unknown";
    }
}