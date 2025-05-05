#ifndef DISPATCHER_H
#define DISPATCHER_H

#include <stdio.h>
#include "../avm.h"

void
dispatcher_initialize(FILE* binaryFile);

void
dispatcher_execute_cycle();

#endif