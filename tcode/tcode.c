#include "../symbol_table/symbol_table.h"
#include "../scope_space/scope_space.h"
#include "../quad/quad.h"

#include "tcode.h"

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

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

instruction* instructions = NULL;
unsigned totalInstructions = 0;
unsigned int currInstruction = 0;

#define EXPAND_SIZE 1024
#define CURR_SIZE (totalInstructions * sizeof(instruction))
#define NEW_SIZE (EXPAND_SIZE * sizeof(instruction) + CURR_SIZE)

#define NUM_CONSTS_SIZE 1024
#define STR_CONSTS_SIZE 1024
#define LIB_FUNCS_SIZE  1024
#define USR_FUNCS_SIZE  1024

double* numConsts = NULL;
unsigned totalNumConsts = 0;

char** stringConsts = NULL;
unsigned totalStringConsts = 0;

char** namedLibFuncs = NULL;
unsigned totalNamedLibFuncs = 0;

userfunc* userFuncs = NULL;
unsigned totalUserFuncs = 0;

typedef void (*generator_func_t)(Quad*);

/* ------------------------------------------ Static Declarations ------------------------------------------ */
static void
expand();

static void
emit(instruction i);

static void
generate(vmopcode op, Quad* quad);

static void
generate_ADD(Quad* q);

static void
generate_SUB(Quad* q);

static void
generate_MUL(Quad* q);

static void
generate_DIV(Quad* q);

static void
generate_MOD(Quad* q);

static void
generate_UMINUS(Quad* q);

static void
make_operand(Expr* e, vmarg* arg);

static void
make_numberOperand(vmarg* arg, double val);

static unsigned
consts_newstring(char* s);

static unsigned
consts_newnumber(double n);

static unsigned
libfuncs_newused(char* s);

static unsigned
userfuncs_newfunc(SymbolTableEntry* entry);

static void
write_instructions(FILE* out);

static const char*
vmopcode_to_string(vmopcode op);

static char*
vmarg_to_string(vmarg arg);

static const char*
vmarg_type_to_string(vmarg_t type);

generator_func_t generators[] = {
    generate_ADD,
    generate_SUB,
    generate_MUL,
    generate_DIV,
    generate_MOD,
    generate_UMINUS,
};

/* ------------------------------------------ Implementation ------------------------------------------ */
unsigned int
tcode_nextInstructionLabel() { return currInstruction; }

void
tcode_printInstructions() {
    write_instructions(stdout);
}

void
tcode_generateInstructions() {
    Quad* quad;
    IOPCodeType op;
    unsigned int totalQuads;

    totalQuads = quad_totalQuads();

    for (unsigned i = 1; i < totalQuads; i++) {
        op = quad_getOpcode(i);
        quad = quad_getAt(i);
        
        (*generators[op])(quad);
    }
}

/* ------------------------------------------ Static Definitions ------------------------------------------ */
static void
expand() {
    assert(totalInstructions == currInstruction);
    instruction* i = malloc(NEW_SIZE);
    if (instructions) {
        memcpy(i, instructions, CURR_SIZE);
        free(instructions);
    }
    instructions = i;
    totalInstructions += EXPAND_SIZE;
}

static void
emit(instruction i) {
    if (currInstruction == totalInstructions) {
        expand();
    }

    instruction* newInstr = instructions + currInstruction++;
    newInstr->arg1 = i.arg1;
    newInstr->arg2 = i.arg2;
    newInstr->result = i.result;
    newInstr->opcode = i.opcode;
    newInstr->srcLine = i.srcLine;
}

static void
generate(vmopcode op, Quad* quad) {

    Expr* arg1;
    Expr* arg2;
    Expr* result;
    instruction instr;

    arg1 = quad_getArg1(quad);
    arg2 = quad_getArg2(quad);
    result = quad_getResult(quad);

    instr.opcode = op;
    instr.srcLine = quad_getLine(quad);

    make_operand(arg1, &instr.arg1);
    make_operand(arg2, &instr.arg2);
    make_operand(result, &instr.result);

    quad_setTargetAddress(quad, currInstruction);

    emit(instr);
}

static void
make_numberOperand(vmarg* arg, double val) {
    arg->val = consts_newnumber(val);
    arg->type = number_a;
}

static void
generate_ADD(Quad* q) { generate(add_v, q); }

static void
generate_SUB(Quad* q) { generate(sub_v, q); }

static void
generate_MUL(Quad* q) { generate(mul_v, q); }

static void
generate_DIV(Quad* q) { generate(div_v, q); }

static void
generate_MOD(Quad* q) { generate(mod_v, q); }

static void
generate_UMINUS(Quad* quad) {

    Expr* arg1;
    Expr* result;
    instruction instr;

    arg1 = quad_getArg1(quad);
    result = quad_getResult(quad);

    instr.opcode = mul_v;
    instr.srcLine = quad_getLine(quad);

    make_operand(arg1, &instr.arg1);
    make_numberOperand(&instr.arg2, -1);
    make_operand(result, &instr.result);

    quad_setTargetAddress(quad, currInstruction);

    emit(instr);
}

static void
make_operand(Expr* e, vmarg* arg) {
    switch (icode_getExprType(e)) {
        case var_e:
        case tableitem_e:
        case arithmexpr_e:
        case boolexpr_e:
        case assignexpr_e:
        case newtable_e: {
            assert(icode_getExprEntry(e));
            
            SymbolTableEntry* entry;
            ScopeSpaceType space;

            unsigned offset;

            entry = icode_getExprEntry(e);
            offset = symtab_getVariableOffset(entry);
            space = symtab_getVariableSpace(entry);

            arg->val = offset;

            switch (space) {
                case PROGRAMVAR: arg->type = global_a; break;
                case FUNCTIONLOCAL: arg->type = local_a; break;
                case FORMALARG: arg->type = formal_a; break;
                default: assert(0);
            }

            break;
        }
        case constbool_e: {
            arg->val = icode_getBoolConst(e);
            arg->type = bool_a;
            break;
        }
        case conststring_e: {
            char* s = icode_getStringConst(e);
            arg->val = consts_newstring(s);
            arg->type = string_a;
            break;
        }
        case constnum_e: {
            double n = icode_getNumConst(e);
            arg->val = consts_newnumber(n);
            arg->type = number_a;
            break;
        }
        case nil_e: {
            arg->type = nil_a;
            break;
        }
        case programfunc_e: {
            SymbolTableEntry* entry = icode_getExprEntry(e);
            arg->val = userfuncs_newfunc(entry);
            arg->type = userfunc_a;
            break;
        }
        case libraryfunc_e: {
            SymbolTableEntry* entry = icode_getExprEntry(e);
            char* name = (char*) symtab_getEntryName(entry);
            arg->val = libfuncs_newused(name);
            arg->type = libfunc_a;
            break;
        }
        default: assert(0);

    }
}

static unsigned
consts_newstring(char* s) {
    if (stringConsts == NULL) {
        stringConsts = malloc(sizeof(char*) * STR_CONSTS_SIZE);

        if (!stringConsts) {
            printf("Error allocating memory for string consts array.\n");
            exit(1);
        }
    }

    if (totalStringConsts == STR_CONSTS_SIZE) {
        printf("Error: Cannot add any more string consts.\n");
        exit(1);
    }

    stringConsts[totalStringConsts++] = strdup(s);
    return totalStringConsts - 1;
}

static unsigned
consts_newnumber(double n) {
    if (numConsts == NULL) {
        numConsts = malloc(sizeof(double) * NUM_CONSTS_SIZE);

        if (!numConsts) {
            printf("Error allocating memory for num consts array.\n");
            exit(1);
        }
    }

    if (totalNumConsts == NUM_CONSTS_SIZE) {
        printf("Error: Cannot add any more num consts.\n");
        exit(1);
    }

    numConsts[totalNumConsts++] = n;
    return totalNumConsts - 1;
}

static unsigned
libfuncs_newused(char* s) {
    if (namedLibFuncs == NULL) {
        namedLibFuncs = malloc(sizeof(char*) * LIB_FUNCS_SIZE);

        if (!namedLibFuncs) {
            printf("Error allocating memory for string consts array.\n");
            exit(1);
        }
    }

    if (totalNamedLibFuncs == LIB_FUNCS_SIZE) {
        printf("Error: Cannot add any more string consts.\n");
        exit(1);
    }

    namedLibFuncs[totalNamedLibFuncs++] = strdup(s);
    return totalNamedLibFuncs - 1;
}

static unsigned
userfuncs_newfunc(SymbolTableEntry* entry) {
    if (userFuncs == NULL) {
        userFuncs = malloc(sizeof(userfunc) * USR_FUNCS_SIZE);

        if (!userFuncs) {
            printf("Error allocating memory for user funcs array.\n");
            exit(1);
        }
    }

    if (totalUserFuncs == USR_FUNCS_SIZE) {
        printf("Error: Cannot add any more user func consts.\n");
        exit(1);
    }

    userFuncs[totalUserFuncs].address = symtab_getFunctionAddress(entry);
    userFuncs[totalUserFuncs].localSize = symtab_getFunctionLocalSize(entry);
    userFuncs[totalUserFuncs].id = (char*) symtab_getEntryName(entry);

    return totalUserFuncs++;
}

static void
write_instructions(FILE* out) {
    fprintf(out, "---------------------------------------------INSTRUCTIONS---------------------------------------------\n");
    fprintf(out, "%-10s %-20s %-20s %-20s %-20s %-10s\n",
         "Instr.", "opcode", "result", "arg1", "arg2", "line");
    fprintf(out, "---------------------------------------------------------------------------------------------------------\n");
    
    for (int i = 0; i < currInstruction; i++) {
        instruction instr = instructions[i];

        fprintf(out, "%-10d %-20s %-20s %-20s %-20s %-10d\n",
            i,
            vmopcode_to_string(instr.opcode),
            vmarg_to_string(instr.result),
            vmarg_to_string(instr.arg1),
            vmarg_to_string(instr.arg2),
            instr.srcLine
        );
    }
    fprintf(out, "\n");
}

static char*
vmarg_to_string(vmarg arg) {
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
        default:           return "unknown";
    }
}