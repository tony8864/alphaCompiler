#ifndef LOADER_H
#define LOADER_H

typedef struct avm_constants avm_constants;

void
loader_openBinaryFile(char* filename);

avm_constants*
loader_load_avm_constants();

#endif