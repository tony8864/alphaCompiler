#ifndef TABLES_H
#define TABLES_H

#include "../avm_types.h"

#define AVM_TABLE_HASHSIZE 211

typedef struct avm_table_bucket avm_table_bucket;
typedef struct avm_table avm_table;

void
execute_tablegetelem(instruction* instr);

void
execute_tablesetelem(instruction* instr);

void
execute_newtable(instruction* instr);

#endif