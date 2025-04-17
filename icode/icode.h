#ifndef ICODE_H
#define ICODE_H

#include "../symbol_table/symbol_table.h"

typedef enum {
    assign_op, add_op, sub_op, mul_op, div_op, mod_op,
    uminus_op, and_op, or_op, not_op, if_eq_op, if_noteq_op,
    if_lesseq_op, if_greatereq_op, if_less_op, if_greater_op,
    call_op, param_op, ret_op, getretval_op, funcstart_op, funcend_op,
    tablecreate_op, tablegetelem_op, tablesetelem_op,
} IOPCodeType;

typedef enum {
    var_e,
    tableitem_e,
    programfunc_e,
    libraryfunc_e,
    arithmexpr_e,
    boolexpr_e,
    assignexpr_e,
    newtable_e,
    constnum_e,
    constbool_e,
    conststring_e,
    nil_e,
} ExprType;

typedef struct Expr Expr;

Expr*
icode_getLvalueExpr(SymbolTableEntry* entry);

#endif