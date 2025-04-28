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

static const char*
get_constBoolExpr_name(Expr* e);

static void
write_quads(FILE* out);

static void
printList(int head);

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

    write_quads(file);
    fclose(file);
}

void quad_printQuads() {
    write_quads(stdout);
}

unsigned int
quad_nextQuadLabel() {
    return currQuad;
}

void
quad_patchLabel(unsigned quadNo, unsigned label) {
    assert(quadNo < currQuad);
    quads[quadNo].label = label;
}

int
quad_newList(int i) {
    quads[i].label = 0;
    return i;
}

int
quad_mergeList(int l1, int l2) {
    if (!l1) {
        return l2;
    }
    else if (!l2) {
        return l1;
    }
    else {
        int i = l1;
        
        while (quads[i].label) {
            i = quads[i].label;
        }
        quads[i].label = l2;
        return l1;
    }
}

void
quad_patchList(int list, int label) {
    while (list) {
        int next = quads[list].label;
        quads[list].label = label;
        list = next;
    }
}

void
quad_printList(int head) {
    if (!head) {
        printf("List is empty.\n");
        return;
    }

    int i = head;
    printf("List: ");
    
    // Optional: track visited nodes to avoid infinite loops
    int count = 0;
    while (i && count < 1000) {
        printf("%d ", i);
        i = quads[i].label;
        count++;
    }

    if (count >= 1000) {
        printf("(Possible cycle detected!)");
    }

    printf("\n");
}
/* ------------------------------ Static Declarations ------------------------------ */
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
        case jump_op:           return "jump";
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
        case arithmexpr_e:
        case assignexpr_e:
        case programfunc_e:
        case libraryfunc_e:
        case boolexpr_e:
        case newtable_e:
            return get_symtab_expr_name(e);
        case conststring_e:
            return get_constStringExpr_name(e);
        case constnum_e:
            return get_constNumExpr_name(e);
        case constbool_e:
            return get_constBoolExpr_name(e);
        case nil_e:
            return "nil";
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
    if (!e) return strdup("");

    char buf[64];
    snprintf(buf, sizeof(buf), "%g", icode_getNumConst(e));
    return strdup(buf);
}

static const char*
get_constBoolExpr_name(Expr* e) {
    if (!e) return "";
    return icode_getBoolConst(e) ? "true" : "false";
}

static void
write_quads(FILE* out) {
    fprintf(out, "%-10s %-20s %-20s %-20s %-20s %-6s %-6s\n",
        "Quad#", "opcode", "result", "arg1", "arg2", "LABEL", "LINE");
    fprintf(out, "---------------------------------------------------------------------------------------------------------\n");

    for (unsigned i = 1; i < currQuad; ++i) {
        Quad q = quads[i];

        const char* arg1_str = get_expr_name(q.arg1);
        const char* arg2_str = get_expr_name(q.arg2);
        const char* result_str = get_expr_name(q.result);
        
        char label_buf[8];
        char line_buf[20];

        if (q.label != 0)
            snprintf(label_buf, sizeof(label_buf), "%u", q.label);
        else
            label_buf[0] = '\0';

        snprintf(line_buf, sizeof(line_buf), "[line %u]", q.line);

        fprintf(out, "%-10d %-20s %-20s %-20s %-20s %-6s %-6s\n",
                i,
                opcode_to_string(q.op),
                result_str,
                arg1_str,
                arg2_str,
                label_buf,
                line_buf
            );
    }
}