#include "../avm_types.h"

#include "memory.h"

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>

avm_memcell stack[AVM_STACKSIZE];
avm_memcell ax, bx, cx;
avm_memcell retval;
unsigned top, topsp;

static void
memclear_string(avm_memcell* m);

static void
memclear_table(avm_memcell* m);

typedef void (*memclear_func_t)(avm_memcell*);

memclear_func_t memclearFuncs[] = {
    0,
    memclear_string,
    0,
    memclear_table,
    0,
    0,
    0,
    0
};

void
memory_initstack(unsigned totalGlobals) {
    top = AVM_STACKSIZE - 1 - totalGlobals;
    for (int i = 0; i < AVM_STACKSIZE; i++) {
        memset(&stack[i], 0, sizeof(stack[i]));
        stack[i].type = undef_m;
    }
}

void avm_memcellclear(avm_memcell* m) {
    if (m->type != undef_m) {
        memclear_func_t f = memclearFuncs[m->type];
        
        if (f) {
            (*f)(m);
        }
        m->type = undef_m;
    }
}

static void
memclear_string(avm_memcell* m) {
    assert(m->data.strVal);
    free(m->data.strVal);
}

static void
memclear_table(avm_memcell* m) {
    // assert(m->data.tableVal);
    // decrement ref counter
}