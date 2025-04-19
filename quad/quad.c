#include "quad.h"

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

typedef struct Quad {
    IOPCodeType op;
    Expr* arg1;
    Expr* arg2;
    Expr* result;
    unsigned label;
    unsigned line;
} Quad;

Quad* quads = NULL;
unsigned total = 0;
unsigned int currQuad = 0;

#define EXPAND_SIZE 1014
#define CURR_SIZE (total * sizeof(Quad))
#define NEW_SIZE (EXPAND_SIZE * sizeof(Quad) + CURR_SIZE)

/* ------------------------------ Static Definitions ------------------------------ */
static void
expand();

static const char*
opcode_to_string(IOPCodeType op);

static const char*
get_expr_name(Expr* e);

static const char*
get_symtab_expr_name(Expr* e);

static const char*
get_constStringExpr_name(Expr* e);

static const char*
get_constNumExpr_name(Expr* e);

/* ------------------------------ Implementation ------------------------------ */
void
quad_emit(IOPCodeType op, Expr* arg1, Expr* arg2, Expr* result, unsigned label, unsigned line) {
    if (currQuad == total) {
        expand();
    }

    Quad* p = quads + currQuad++;
    p->arg1 = arg1;
    p->arg2 = arg2;
    p-> result = result;
    p->label = label;
    p->line = line;
    p->op = op;
}

void
quad_writeQuadsToFile(char* filename) {
    FILE* file;
    Quad q;

    file = fopen(filename, "w");

    if (!file) {
        printf("Error opening file to write quads.\n");
        exit(1);
    }

    for (int i = 0; i < currQuad; i++) {
        q = quads[i];

        const char* opcode = opcode_to_string(q.op);
        const char* arg1 = get_expr_name(q.arg1);
        const char* arg2 = get_expr_name(q.arg2);
        const char* result = get_expr_name(q.result);

        //fprintf(file, "%-10s %-10s %-10s %-10s", opcode, result, arg1, arg2);

        fprintf(file, "%s %s %s %s", opcode, result, arg1, arg2);

        if (q.label != 0) {
            fprintf(file, "%u", q.label);
        }
        fprintf(file, "\n");
    }
}

void quad_printQuads() {
    printf("%-5s | %-12s | %-10s | %-10s | %-20s | %-6s | %-5s\n",
           "ID", "OP", "ARG1", "ARG2", "RESULT", "LABEL", "LINE");
    printf("----------------------------------------------------------------------------\n");

    for (unsigned i = 0; i < currQuad; ++i) {
        Quad q = quads[i];

        const char* arg1_str = get_expr_name(q.arg1);
        const char* arg2_str = get_expr_name(q.arg2);
        const char* result_str = get_expr_name(q.result);
        
        char label_buf[8];

        if (q.label != 0)
            snprintf(label_buf, sizeof(label_buf), "%u", q.label);
        else
            label_buf[0] = '\0';

        printf("%-5u | %-12s | %-10s | %-10s | %-20s | %-6s | %-5u\n",
               i,
               opcode_to_string(q.op),
               arg1_str,
               arg2_str,
               result_str,
               label_buf,
               q.line);
    }
}

static void
expand() {
    assert(total == currQuad);
    Quad* p = malloc(NEW_SIZE);
    if (quads) {
        memcpy(p, quads, CURR_SIZE);
        free(quads);
    }
    quads = p;
    total += EXPAND_SIZE;
}

/* ------------------------------ Static Declarations ------------------------------ */
static const char*
opcode_to_string(IOPCodeType op) {
    switch (op) {
        case assign_op:         return "assign";
        case add_op:            return "add";
        case sub_op:            return "sub";
        case mul_op:            return "mul";
        case div_op:            return "div";
        case mod_op:            return "mod";
        case uminus_op:         return "uminus";
        case and_op:            return "and";
        case or_op:             return "or";
        case not_op:            return "not";
        case if_eq_op:          return "if_eq";
        case if_noteq_op:       return "if_neq";
        case if_lesseq_op:      return "if_less_eq";
        case if_greatereq_op:   return "if_greater_eq";
        case if_less_op:        return "if_less";
        case if_greater_op:     return "if_greater";
        case call_op:           return "call";
        case param_op:          return "param";
        case ret_op:            return "ret";
        case getretval_op:      return "getretval";
        case funcstart_op:      return "funcstart";
        case funcend_op:        return "funcend";
        case tablecreate_op:    return "tablecreate";
        case tablegetelem_op:   return "tablegetelem";
        case tablesetelem_op:   return "tablesetelem";
        default:                return "unknown";
    }
}

static const char*
get_expr_name(Expr* e) {
    if (!e) {
        return "";
    }

    switch (icode_getExprType(e)) {
        case var_e:
        case tableitem_e:
        case assignexpr_e:
        case programfunc_e:
            return get_symtab_expr_name(e);
        case conststring_e:
            return get_constStringExpr_name(e);
        case constnum_e:
            return get_constNumExpr_name(e);
        default:
            assert(0);
    }
}

static const char*
get_symtab_expr_name(Expr* e) {
    if (!e) return "";
    SymbolTableEntry* entry = icode_getExprEntry(e);
    return entry ? symtab_getEntryName(entry) : "";
}

static const char*
get_constStringExpr_name(Expr* e) {
    if (!e) return "";
    return icode_getStringConst(e);
}

static const char*
get_constNumExpr_name(Expr* e) {
    static char buf[64];
    if (!e) return "";

    double i = icode_getNumConst(e);
    snprintf(buf, sizeof(buf), "%g", i);
    return buf;
}