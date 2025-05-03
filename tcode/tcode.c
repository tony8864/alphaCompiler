#include "../symbol_table/symbol_table.h"
#include "../scope_space/scope_space.h"
#include "../func_stack/func_stack.h"
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

typedef struct IncompleteJump {
    unsigned instrNo;
    unsigned iaddress;
    struct IncompleteJump* next;
} IncompleteJump;

IncompleteJump* ij_head = NULL;
unsigned ij_total = 0;

instruction* instructions = NULL;
unsigned totalInstructions = 0;
unsigned int currInstruction = 0;

FuncStack* funcStack = NULL;

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
generate_relational(vmopcode op, Quad* quad);

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
generate_NEWTABLE(Quad* q);

static void
generate_TABLEGETELEM(Quad* q);

static void
generate_TABLESETELEM(Quad* q);

static void
generate_ASSIGN(Quad* q);

static void
generate_JUMP(Quad* q);

static void
generate_IF_EQ(Quad* q);

static void
generate_IF_NOTEQ(Quad* q);

static void
generate_IF_GREATER(Quad* q);

static void
generate_IF_GREATEREQ(Quad* q);

static void
generate_IF_LESS(Quad* q);

static void
generate_IF_LESSEQ(Quad* q);

static void
generate_NOT(Quad* q);

static void
generate_OR(Quad* q);

static void
generate_AND(Quad* q);

static void
generate_PARAM(Quad* q);

static void
generate_CALL(Quad* q);

static void
generate_GETRETVAL(Quad* q);

static void
generate_FUNCSTART(Quad* q);

static void
generate_RETURN(Quad* q);

static void
generate_FUNCEND(Quad* q);

static void
make_operand(Expr* e, vmarg* arg);

static void
make_numberOperand(vmarg* arg, double val);

static void
make_boolOperand(vmarg* arg, unsigned val);

static void
make_retvalOperand(vmarg* arg);

static void
reset_operand(vmarg* arg);

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

static void
add_incomplete_jump(unsigned instrNo, unsigned iaddress);

static void
patch_incomplete_jumps();

static void
backpatch(RetList* rlist);

static void
writeMagicNumber(FILE* file);

static void
writeArrays(FILE* file);

static void
writeStringArray(FILE* file);

static void
writeNumberArray(FILE* file);

static void
writeUserFuncArray(FILE* file);

static void
writeLibFuncArray(FILE* file);

static void
writeCode(FILE* file);

generator_func_t generators[] = {
    generate_ADD,
    generate_SUB,
    generate_MUL,
    generate_DIV,
    generate_MOD,
    generate_UMINUS,
    generate_ASSIGN,
    generate_NEWTABLE,
    generate_TABLEGETELEM,
    generate_TABLESETELEM,
    generate_JUMP,
    generate_IF_EQ,
    generate_IF_NOTEQ,
    generate_IF_GREATER,
    generate_IF_GREATEREQ,
    generate_IF_LESS,
    generate_IF_LESSEQ,
    generate_NOT,
    generate_OR,
    generate_AND,
    generate_PARAM,
    generate_CALL,
    generate_GETRETVAL,
    generate_FUNCSTART,
    generate_RETURN,
    generate_FUNCEND
};

/* ------------------------------------------ Implementation ------------------------------------------ */
void
tcode_initialize() {
    funcStack = funcStack_initialize();
}

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

    patch_incomplete_jumps();
}

void
tcode_printUserFuncs() {
    userfunc f;
    printf("User Funcs:\n");
    for (int i = 0; i < totalUserFuncs; i++) {
        f = userFuncs[i];
        printf("[%s, %u] ", f.id, f.address);
    }
    printf("\n\n");
}

void
tcode_printNamedLibs() {
    printf("Named libs:\n");
    for (int i = 0; i < totalNamedLibFuncs; i++) {
        printf("%d: %s\n", i, namedLibFuncs[i]);
    }
    printf("\n");
}

void
tcode_printStringConsts() {
    printf("String consts:\n");
    for (int i = 0; i < totalStringConsts; i++) {
        printf("%d: %s\n", i, stringConsts[i]);
    }
    printf("\n");
}

void
tcode_printNumConsts() {
    printf("Num consts:\n");
    for (int i =0; i < totalNumConsts; i++) {
        printf("%d: %f\n", i, numConsts[i]);
    }
    printf("\n");
}

void
tcode_createBinaryFile(char* filename) {
    FILE* file;

    file = fopen(filename, "w");

    if (!file) {
        printf("Error opening file to write instructions.\n");
        exit(1);
    }

    writeMagicNumber(file);
    writeArrays(file);
    writeCode(file);

    fclose(file);
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
generate_relational(vmopcode op, Quad* quad) {
    
    Expr* arg1;
    Expr* arg2;
    instruction instr;
    unsigned quad_label;
    int quad_index;

    arg1 = quad_getArg1(quad);
    arg2 = quad_getArg2(quad);
    quad_label = quad_getLabel(quad);
    quad_index = quad_getIndex(quad);

    instr.opcode = op;
    instr.result.type = label_a;
    instr.srcLine = quad_getLine(quad);

    make_operand(arg1, &instr.arg1);
    make_operand(arg2, &instr.arg2);

    if (quad_label < quad_index) {
        Quad* target_quad = quad_getAt(quad_label);
        instr.result.val = quad_getTargetAddress(target_quad);
    }
    else {
        add_incomplete_jump(currInstruction, quad_label);
    }

    quad_setTargetAddress(quad, currInstruction);

    emit(instr);
}

static void
make_numberOperand(vmarg* arg, double val) {
    arg->val = consts_newnumber(val);
    arg->type = number_a;
}

static void
make_boolOperand(vmarg* arg, unsigned val) {
    arg->val = val;
    arg->type = bool_a;
}

static void
make_retvalOperand(vmarg* arg) {
    arg->type = retval_a;
    arg->val = 10;
}

static void
reset_operand(vmarg* arg) {
    arg->type = notype_a;
    arg->val = 0;
}

static void generate_ADD(Quad* q) { generate(add_v, q); }
static void generate_SUB(Quad* q) { generate(sub_v, q); }
static void generate_MUL(Quad* q) { generate(mul_v, q); }
static void generate_DIV(Quad* q) { generate(div_v, q); }
static void generate_MOD(Quad* q) { generate(mod_v, q); }

static void generate_ASSIGN(Quad* q)        { generate(assign_v, q); } 
static void generate_NEWTABLE(Quad* q)      { generate(newtable_v, q); }
static void generate_TABLEGETELEM(Quad* q)  { generate(tablegetelem_v, q); }
static void generate_TABLESETELEM(Quad* q)  { generate(tablesetelem_v, q); }

static void generate_JUMP(Quad* q)          { generate_relational(jump_v, q); }
static void generate_IF_EQ(Quad* q)         { generate_relational(jeq_v, q); }
static void generate_IF_NOTEQ(Quad* q)      { generate_relational(jne_v, q); }
static void generate_IF_GREATER(Quad* q)    { generate_relational(jgt_v, q); }
static void generate_IF_GREATEREQ(Quad* q)  { generate_relational(jge_v, q); }
static void generate_IF_LESS(Quad* q)       { generate_relational(jlt_v, q); }
static void generate_IF_LESSEQ(Quad* q)     { generate_relational(jle_v, q); }

static void
generate_NOT(Quad* q) {
    Expr* arg1;
    Expr* result;
    instruction instr;

    arg1 = quad_getArg1(q);
    result = quad_getResult(q);

    quad_setTargetAddress(q, currInstruction);

    instr.srcLine = quad_getLine(q);

    // 1st emit
    instr.opcode = jeq_v;
    instr.result.type = label_a;
    instr.result.val = currInstruction + 3;
    make_operand(arg1, &instr.arg1);
    make_boolOperand(&instr.arg2, 0);
    emit(instr);

    // 2nd emit
    instr.opcode = assign_v;
    reset_operand(&instr.arg2);
    make_boolOperand(&instr.arg1, 0);
    make_operand(result, &instr.result);
    emit(instr);

    // 3rd emit
    instr.opcode = jump_v;
    instr.result.type = label_a;
    instr.result.val = currInstruction + 2;
    reset_operand(&instr.arg1);
    reset_operand(&instr.arg2);
    emit(instr);

    // 4th emit
    instr.opcode = assign_v;
    reset_operand(&instr.arg2);
    make_boolOperand(&instr.arg1, 1);
    make_operand(result, &instr.result);
    emit(instr);
}

static void
generate_OR(Quad* q) {
    Expr* arg1;
    Expr* arg2;
    Expr* result;
    instruction instr;

    arg1 = quad_getArg1(q);
    arg2 = quad_getArg2(q);
    result = quad_getResult(q);

    quad_setTargetAddress(q, currInstruction);

    instr.srcLine = quad_getLine(q);

    // 1st emit
    instr.opcode = jeq_v;
    instr.result.type = label_a;
    instr.result.val = currInstruction + 4;
    make_operand(arg1, &instr.arg1);
    make_boolOperand(&instr.arg2, 1);
    emit(instr);

    // 2nd emit
    instr.result.val = currInstruction + 3;
    make_operand(arg2, &instr.arg1);
    emit(instr);

    // 3rd emit
    instr.opcode = assign_v;
    reset_operand(&instr.arg2);
    make_boolOperand(&instr.arg1, 0);
    make_operand(result, &instr.result);
    emit(instr);

    // 4th emit
    instr.opcode = jump_v;
    instr.result.type = label_a;
    instr.result.val = currInstruction + 2;
    reset_operand(&instr.arg1);
    reset_operand(&instr.arg2);
    emit(instr);

    // 5th emit
    instr.opcode = assign_v;
    make_boolOperand(&instr.arg1, 1);
    make_operand(result, &instr.result);
    emit(instr);
}

static void
generate_AND(Quad* q) {
    Expr* arg1;
    Expr* arg2;
    Expr* result;
    instruction instr;

    arg1 = quad_getArg1(q);
    arg2 = quad_getArg2(q);
    result = quad_getResult(q);

    quad_setTargetAddress(q, currInstruction);

    instr.srcLine = quad_getLine(q);

    // 1st emit
    instr.opcode = jeq_v;
    instr.result.type = label_a;
    instr.result.val = currInstruction + 4;
    make_operand(arg1, &instr.arg1);
    make_boolOperand(&instr.arg2, 0);
    emit(instr);

    // 2nd emit
    instr.result.val = currInstruction + 3;
    make_operand(arg2, &instr.arg1);
    emit(instr);

    // 3rd emit
    instr.opcode = assign_v;
    reset_operand(&instr.arg2);
    make_boolOperand(&instr.arg1, 1);
    make_operand(result, &instr.result);
    emit(instr);

    // 4th emit
    instr.opcode = jump_v;
    instr.result.type = label_a;
    instr.result.val = currInstruction + 2;
    reset_operand(&instr.arg1);
    emit(instr);

    // 5th emit
    instr.opcode = assign_v;
    reset_operand(&instr.arg2);
    make_boolOperand(&instr.arg1, 0);
    make_operand(result, &instr.result);
    emit(instr);
}

static void
generate_PARAM(Quad* q) {
    Expr* arg1;
    instruction instr;

    arg1 = quad_getArg1(q);

    instr.srcLine = quad_getLine(q);
    instr.opcode = pusharg_v;
    make_operand(arg1, &instr.arg1);
    reset_operand(&instr.arg2);
    reset_operand(&instr.result);

    quad_setTargetAddress(q, currInstruction);

    emit(instr);
}

static void
generate_CALL(Quad* q) {
    Expr* arg1;
    instruction instr;

    arg1 = quad_getArg1(q);

    instr.srcLine = quad_getLine(q);
    instr.opcode = call_v;
    make_operand(arg1, &instr.arg1);

    quad_setTargetAddress(q, currInstruction);

    emit(instr);
}

static void
generate_GETRETVAL(Quad* q) {
    Expr* result;
    instruction instr;

    result = quad_getResult(q);

    instr.srcLine = quad_getLine(q);
    instr.opcode = assign_v;
    make_operand(result, &instr.result);
    make_retvalOperand(&instr.arg1);

    quad_setTargetAddress(q, currInstruction);
    
    emit(instr);
}

static void
generate_FUNCSTART(Quad* q) {
    Expr* arg1;
    SymbolTableEntry* entry;
    instruction instr;

    arg1 = quad_getArg1(q);
    entry = icode_getExprEntry(arg1);

    symtab_setFunctionAddress(entry, currInstruction);

    funcStack_pushList(funcStack);

    instr.srcLine = quad_getLine(q);
    instr.opcode = funcenter_v;
    make_operand(arg1, &instr.arg1);
    reset_operand(&instr.arg2);
    reset_operand(&instr.result);

    quad_setTargetAddress(q, currInstruction);

    emit(instr);
}

static void
generate_RETURN(Quad* q) {
    RetList* rlist;
    Expr* result;
    instruction instr;

    result = quad_getResult(q);

    // 1st emit
    instr.srcLine = quad_getLine(q);
    instr.opcode = assign_v;
    make_retvalOperand(&instr.result);
    make_operand(result, &instr.arg1);
    reset_operand(&instr.arg2);
    
    quad_setTargetAddress(q, currInstruction);

    emit(instr);

    rlist = funcStack_top(funcStack);
    funcStack_appendToReturnList(rlist, currInstruction);

    // 2nd emit
    instr.opcode = jump_v;
    reset_operand(&instr.arg1);
    reset_operand(&instr.arg2);
    instr.result.type = label_a;
    instr.result.val = 0;
    emit(instr);
}

static void
generate_FUNCEND(Quad* q) {
    RetList* rlist;
    Expr* arg1;
    instruction instr;

    arg1 = quad_getArg1(q);

    rlist = funcStack_pop(funcStack);
    backpatch(rlist);

    instr.srcLine = quad_getLine(q);
    instr.opcode = funcexit_v;
    reset_operand(&instr.result);
    reset_operand(&instr.arg2);
    make_operand(arg1, &instr.arg1);

    quad_setTargetAddress(q, currInstruction);

    emit(instr);

    funcStack_freeRetList(rlist);
}

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

    if (e == NULL) {
        arg->val = 0;
        arg->type = notype_a;
        return;
    }

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

    unsigned int address;
    char* id;

    id = (char*) symtab_getEntryName(entry);
    address = symtab_getFunctionAddress(entry);

    for (int i = 0; i < totalUserFuncs; i++) {
        if (!strcmp(userFuncs[i].id, id) && userFuncs[i].address == address) {
            return i;
        }
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

static void
add_incomplete_jump(unsigned instrNo, unsigned iaddress) {
    IncompleteJump* newJump;
    IncompleteJump* curr = ij_head;
    IncompleteJump* prev = NULL;

    while (curr) {
        prev = curr;
        curr = curr->next;
    }

    newJump = malloc(sizeof(IncompleteJump));
    newJump->instrNo = instrNo;
    newJump->iaddress = iaddress;
    newJump->next = NULL;

    if (prev == NULL) {
        ij_head = newJump;
    }
    else {
        prev->next = newJump;
    }

    ij_total += 1;
}

static void
patch_incomplete_jumps() {
    IncompleteJump* curr = ij_head;
    unsigned instrNo;
    unsigned iaddress;

    while (curr) {
        iaddress = curr->iaddress;
        instrNo = curr->instrNo;

        if (iaddress == quad_nextQuadLabel()) {
            instructions[instrNo].result.val = currInstruction;
        }
        else {
            Quad* q = quad_getAt(iaddress);
            instructions[instrNo].result.val = quad_getTargetAddress(q);
        }

        curr = curr->next;
    }
}

static void
backpatch(RetList* rlist) {
    funcStack_printRetList(rlist);
    RetNode* curr;
    unsigned int ret;

    curr = funcStack_getRetNodeHead(rlist);

    while (curr) {
        ret = funcStack_getRetNodeValue(curr);
        instructions[ret].result.val = currInstruction;
        curr = funcStack_getRetNodeNext(curr);
    }
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

static void
writeMagicNumber(FILE* file) {
    unsigned magicnumber = 340200501;
    fprintf(file, "%u\n", magicnumber);
}

static void
writeArrays(FILE* file) {
    writeStringArray(file);
    writeNumberArray(file);
    writeUserFuncArray(file);
    writeLibFuncArray(file);
}

static void
writeStringArray(FILE* file) {
    char* str;
    fprintf(file, "%d ", totalStringConsts);
    for (int i = 0; i < totalStringConsts; i++) {
        str = stringConsts[i];
        fprintf(file, "%d %s ", (int) strlen(str), str);
    }
    fprintf(file, "\n");
}

static void
writeNumberArray(FILE* file) {
    double num;
    fprintf(file, "%d ", totalNumConsts);
    for (int i = 0; i < totalNumConsts; i++) {
        num = numConsts[i];
        fprintf(file, "%f ", num);
    }
    fprintf(file, "\n");
}

static void
writeUserFuncArray(FILE* file) {
    userfunc f;
    unsigned addr;
    unsigned localSize;
    char* id;
    fprintf(file, "%d ", totalUserFuncs);
    for (int i = 0; i < totalUserFuncs; i++) {
        f = userFuncs[i];
        addr = f.address;
        localSize = f.localSize;
        id = f.id;
        fprintf(file, "%u %u %d %s ", addr, localSize, (int) strlen(id) ,id);
    }
    fprintf(file, "\n");
}

static void
writeLibFuncArray(FILE* file) {
    char* lib;
    fprintf(file, "%d ", totalNamedLibFuncs);
    for (int i = 0; i < totalNamedLibFuncs; i++) {
        lib = namedLibFuncs[i];
        fprintf(file, "%d %s ", (int) strlen(lib), lib);
    }
    fprintf(file, "\n");
}

static void
writeCode(FILE* file) {
    instruction instr;
    vmarg result;
    vmarg arg1;
    vmarg arg2;
    fprintf(file, "%u\n", currInstruction);
    for (int i = 0; i < currInstruction; i++) {
        instr = instructions[i];
        result = instr.result;
        arg1 = instr.arg1;
        arg2 = instr.arg2;
        fprintf(file, "%u ", instr.opcode);
        fprintf(file, "%u %u ", result.type, result.val);
        fprintf(file, "%u %u ", arg1.type, arg1.val);
        fprintf(file, "%u %u\n", arg2.type, arg2.val);
    }
}