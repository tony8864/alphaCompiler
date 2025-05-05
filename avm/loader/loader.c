#include "loader.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

FILE* binaryFile = NULL;

double* numConsts = NULL;
unsigned totalNumConsts = 0;

char** stringConsts = NULL;
unsigned totalStringConsts = 0;

char** namedLibFuncs = NULL;
unsigned totalNamedLibFuncs = 0;

userfunc* userFuncs = NULL;
unsigned totalUserFuncs = 0;

instruction* instructions = NULL;
unsigned codeSize;

unsigned int totalGlobals;

/* ------------------------------------ Static Declarations ------------------------------------ */
static void
readMagicNUmber();

static void
loadArrays();

static void
loadStringArray();

static void
loadNumArray();

static void
loadUserFuncs();

static void
loadLibFuncs();

static void
loadInstructions();

static vmopcode
readVmopcode();

static void
readVmarg(vmarg* arg);

static void
loadTotalGlobals();

static void
printStringArray();

static void
printNumArray();

static void
printUserFuncs();

static void
printLibFuncs();

static void
print_instructions();

static void
print_totalGlobals();

static const char*
vmopcode_to_string(vmopcode op);

static char*
vmarg_to_string(vmarg arg);

static const char*
vmarg_type_to_string(vmarg_t type);

/* ------------------------------------ Implementation ------------------------------------ */
void
loader_loadBinaryFile(FILE* file) {
    assert(file);
    binaryFile = file;
}

void
loader_loadCode() {
    readMagicNUmber();
    loadArrays();
    loadInstructions();
    loadTotalGlobals();

    // printStringArray();
    // printNumArray();
    // printUserFuncs();
    // printLibFuncs();
    // print_instructions();
    // print_totalGlobals();
}

double*
loader_getNumConsts() {
    assert(numConsts);
    return numConsts;
}

char**
loader_getStringConsts() {
    assert(stringConsts);
    return stringConsts;
}

char**
loader_getLibFuncs() {
    assert(namedLibFuncs);
    return namedLibFuncs;
}

userfunc*
loader_getUserFuncs() {
    assert(userFuncs);
    return userFuncs;
}

instruction*
loader_getInstructions() {
    assert(instructions);
    return instructions;
}

unsigned
loader_getCodeSize() {
    return codeSize;
}

/* ------------------------------------ Static Definitions ------------------------------------ */
static void
readMagicNUmber() {
    int number;
    if (fscanf(binaryFile, "%d", &number) != 1) {
        printf("Error reading magic number from binary file.\n");
        exit(EXIT_FAILURE);
    }

    if (number != 340200501) {
        printf("Error: Magic number is incorrect.\n");
        exit(1);
    }
}

static void
loadArrays() {
    loadStringArray();
    loadNumArray();
    loadUserFuncs();
    loadLibFuncs();
}

static void
loadStringArray() {
    if (fscanf(binaryFile, "%u", &totalStringConsts) != 1) {
        printf("Error reading total string consts from binary file.\n");
        exit(1);
    }

    stringConsts = malloc(sizeof(char*) * totalStringConsts);
    if (!stringConsts) {
        printf("Error allocating memory for string consts array.\n");
        exit(1);
    }

    int len;
    int ch;
    for (int i = 0; i < totalStringConsts; i++) {
        if (fscanf(binaryFile, "%d", &len) != 1) {
            printf("Error reading length of string. %d\n", len);
            exit(1);
        }

        // consume space after strings length
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
}

static void
loadNumArray() {
    if (fscanf(binaryFile, "%u", &totalNumConsts) != 1) {
        printf("Error reading total num consts from binary file.\n");
        exit(1);
    }

    numConsts = malloc(sizeof(double) * totalNumConsts);
    if (!numConsts) {
        printf("Error allocating memory for num consts.\n");
        exit(1);
    }

    int len;
    double n;
    for (int i = 0; i < totalNumConsts; i++) {
        if (fscanf(binaryFile, "%lf", &n) != 1) {
            printf("Error reading num const.\n");
            exit(1);
        }

        numConsts[i] = n;
    }
}

static void
loadUserFuncs() {
    if (fscanf(binaryFile, "%u", &totalUserFuncs) != 1) {
        printf("Error reading total user funcs from binary file.\n");
        exit(1);
    }

    userFuncs = malloc(sizeof(userfunc) * totalUserFuncs);
    if (!userFuncs) {
        printf("Error allocating memory for user funcs.\n");
        exit(1);
    }

    userfunc f;
    unsigned address;
    unsigned localSize;
    char* id;
    int len;
    char ch;
    for (int i = 0; i < totalUserFuncs; i++) {
        if (fscanf(binaryFile, "%u", &address) != 1) {
            printf("Error reading address of user function.\n");
            exit(1);
        }

        if (fscanf(binaryFile, "%u", &localSize) != 1) {
            printf("Error reading local size of user function.\n");
            exit(1);
        }

        if (fscanf(binaryFile, "%d", &len) != 1) {
            printf("Error reading length of user func. %d\n", len);
            exit(1);
        }

        // consume space after reading user func len
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

        userFuncs[i].address = address;
        userFuncs[i].localSize = localSize;
        userFuncs[i].id = id;
    }
}

static void
loadLibFuncs() {
    if (fscanf(binaryFile, "%u", &totalNamedLibFuncs) != 1) {
        printf("Error reading total string consts from binary file.\n");
        exit(1);
    }

    namedLibFuncs = malloc(sizeof(char*) * totalNamedLibFuncs);
    if (!namedLibFuncs) {
        printf("Error allocating memory for string consts array.\n");
        exit(1);
    }

    int len;
    int ch;
    for (int i = 0; i < totalNamedLibFuncs; i++) {
        if (fscanf(binaryFile, "%d", &len) != 1) {
            printf("Error reading length of string. %d\n", len);
            exit(1);
        }

        // consume space after strings length
        fgetc(binaryFile);

        namedLibFuncs[i] = malloc(sizeof(char) * (len + 1));
        if (!namedLibFuncs[i]) {
            printf("Error allocating memory for const string.\n");
            exit(1);
        }

        for (int j = 0; j < len; j++) {
            ch = fgetc(binaryFile);
            if (ch == EOF) {
                printf("Error: Unexpected EOF while reading string.\n");
                exit(1);
            }

            namedLibFuncs[i][j] = (char) ch;
        }

        namedLibFuncs[i][len] = '\0';
    }
}

static void
loadInstructions() {
    if (fscanf(binaryFile, "%u", &codeSize) != 1) {
        printf("Error reading code size from binary file.\n");
        exit(1);
    }

    instructions = malloc(sizeof(instruction) * codeSize);
    if (!instructions) {
        printf("Error allocating memory for instructions.\n");
        exit(1);
    }

    vmarg result;
    vmarg arg1;
    vmarg arg2;
    vmopcode op;
    for (int i = 0; i < codeSize; i++) {
        op = readVmopcode();
        readVmarg(&result);
        readVmarg(&arg1);
        readVmarg(&arg2);

        instructions[i].opcode = op;
        instructions[i].result = result;
        instructions[i].arg1 = arg1;
        instructions[i].arg2 = arg2;
    }
}

static vmopcode
readVmopcode() {
    vmopcode op;
    if (fscanf(binaryFile, "%u", &op) != 1) {
        printf("Error reading opcode from binary file.\n");
        exit(1);
    }
    return op;
}

static void
readVmarg(vmarg* arg) {
    vmarg_t type;
    unsigned value;
    if (fscanf(binaryFile, "%u", &type) != 1) {
        printf("Error reading vmarg type from binary file.\n");
        exit(1);
    }

    if (fscanf(binaryFile, "%u", &value) != 1) {
        printf("Error reading vmarg value from binary file.\n");
        exit(1);
    }

    arg->type = type;
    arg->val = value;
}

static void
loadTotalGlobals() {
    if (fscanf(binaryFile, "%u", &totalGlobals) != 1) {
        printf("Error reading total globals from binary file.\n");
        exit(1);
    }
}

static void
print_totalGlobals() {
    printf("total globals: %u\n", totalGlobals);
}

static void
printStringArray() {
    char* str;
    for (int i = 0; i < totalStringConsts; i++) {
        str = stringConsts[i];
        printf("%d: %s\n", i, str);
    }
}

static void
printNumArray() {
    double n;
    for (int i = 0; i < totalNumConsts; i++) {
        n = numConsts[i];
        printf("%d: %f\n", i, numConsts[i]);
    }
}

static void
printUserFuncs() {
    userfunc f;
    for (int i = 0; i < totalUserFuncs; i++) {
        f = userFuncs[i];
        printf("%u %u %s\n", f.address, f.localSize, f.id);
    }
}

static void
printLibFuncs() {
    char* lib;
    for (int i = 0; i < totalNamedLibFuncs; i++) {
        lib = namedLibFuncs[i];
        printf("%d: %s\n", i, lib);
    }
}

static void
print_instructions() {
    fprintf(stdout, "---------------------------------------------INSTRUCTIONS---------------------------------------------\n");
    fprintf(stdout, "%-10s %-20s %-20s %-20s %-20s %-10s\n",
         "Instr.", "opcode", "result", "arg1", "arg2", "line");
    fprintf(stdout, "---------------------------------------------------------------------------------------------------------\n");
    
    for (int i = 0; i < codeSize; i++) {
        instruction instr = instructions[i];

        fprintf(stdout, "%-10d %-20s %-20s %-20s %-20s %-10d\n",
            i,
            vmopcode_to_string(instr.opcode),
            vmarg_to_string(instr.result),
            vmarg_to_string(instr.arg1),
            vmarg_to_string(instr.arg2),
            instr.srcLine
        );
    }
    fprintf(stdout, "\n");
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