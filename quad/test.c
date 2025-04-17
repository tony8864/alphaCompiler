#include "quad.h"
#include <stdio.h>

int main() {
    // Emit quads with NULL args and result
    quad_emit(assign_op, NULL, NULL, NULL, 0, 1);
    quad_emit(add_op, NULL, NULL, NULL, 0, 2);
    quad_emit(if_eq_op, NULL, NULL, NULL, 10, 3);
    quad_emit(call_op, NULL, NULL, NULL, 0, 4);
    quad_emit(ret_op, NULL, NULL, NULL, 0, 5);

    // Print all quads
    quad_printQuads();

    return 0;
}
