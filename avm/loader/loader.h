#ifndef LOADER_H
#define LOADER_H

typedef struct avm_constants avm_constants;

avm_constants*
loader_load_avm_constants(char* filename);

double
loader_consts_getnumber(avm_constants* consts, unsigned index);

userfunc
loader_consts_getuserfunc(avm_constants* consts, unsigned index);

unsigned
loader_getTotalGlobals(avm_constants* consts);

instruction*
loader_getcode(avm_constants* consts);

unsigned
loader_getcodeSize(avm_constants* consts);

#endif