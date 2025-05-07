#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "avm_types.h"

#include "loader/loader.h"
#include "memory/memory.h"
#include "dispatcher/dispatcher.h"

#define consts_number(index) loader_consts_getnumber(consts, index)

static avm_constants* consts;

unsigned codeSize = 0;
instruction* code = NULL;

static void
loader_init(char* binFilename);

int main(int argc, char** argv) {

    char* binFilename;

    if (argc <= 1) {
        printf("You must provide the path of the binary file.\n");
        exit(1);
    }

    binFilename = argv[1];

    memory_initstack();
    loader_init(binFilename);

    while (!isExecutionFinished()) {
        execute_cycle();
    }

    return 0;
}

static void
loader_init(char* binFilename) {
    consts   = loader_load_avm_constants(binFilename);
    code     = loader_getcode(consts);
    codeSize = loader_getcodeSize(consts);
}

avm_memcell* avm_translate_operand(vmarg* arg, avm_memcell* reg) {
    switch (arg->type) {
        case global_a: {
            return &stack[AVM_STACKSIZE - 1 - arg->val];
        }
        case local_a: break;
        case formal_a:  break;
        case retval_a: break;
        case number_a: {
            reg->type = number_m;
            reg->data.numVal = consts_number(arg->val);
            return reg;
        }
        case string_a: break;
        case bool_a: {
            reg->type = bool_m;
            reg->data.boolVal = arg->val;
            return reg;
        }
        case nil_a: break;
        case userfunc_a: break;
        case libfunc_a: break;
        default: assert(0);
    }
}

void avm_warning(char* str) {
    printf("AVM Warining: %s\n", str);
}