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

void quad_printQuads() {
    printf("%-5s | %-12s | %-10s | %-10s | %-10s | %-6s | %-5s\n",
           "ID", "OP", "ARG1", "ARG2", "RESULT", "LABEL", "LINE");
    printf("----------------------------------------------------------------------------\n");

    for (unsigned i = 0; i < currQuad; ++i) {
        Quad q = quads[i];

        const char* arg1_str = (q.arg1) ? "EXPR" : "";
        const char* arg2_str = (q.arg2) ? "EXPR" : "";
        const char* result_str = (q.result) ? "EXPR" : "";
        char label_buf[8];

        if (q.label != 0)
            snprintf(label_buf, sizeof(label_buf), "%u", q.label);
        else
            label_buf[0] = '\0';

        printf("%-5u | %-12s | %-10s | %-10s | %-10s | %-6s | %-5u\n",
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
        case assign_op:         return "ASSIGN";
        case add_op:            return "ADD";
        case sub_op:            return "SUB";
        case mul_op:            return "MUL";
        case div_op:            return "DIV";
        case mod_op:            return "MOD";
        case uminus_op:         return "UMINUS";
        case and_op:            return "AND";
        case or_op:             return "OR";
        case not_op:            return "NOT";
        case if_eq_op:          return "IF_EQ";
        case if_noteq_op:       return "IF_NEQ";
        case if_lesseq_op:      return "IF_LESS_EQ";
        case if_greatereq_op:   return "IF_GREATER_EQ";
        case if_less_op:        return "IF_LESS";
        case if_greater_op:     return "IF_GREATER";
        case call_op:           return "CALL";
        case param_op:          return "PARAM";
        case ret_op:            return "RET";
        case getretval_op:      return "GETRETVAL";
        case funcstart_op:      return "FUNCSTART";
        case funcend_op:        return "FUNCEND";
        case tablecreate_op:    return "TABLECREATE";
        case tablegetelem_op:   return "TABLEGETELEM";
        case tablesetelem_op:   return "TABLESETELEM";
        default:                return "UNKNOWN";
    }
}
