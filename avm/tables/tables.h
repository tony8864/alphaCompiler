#ifndef TABLES_H
#define TABLES_H

#include "../avm_types.h"

#define AVM_TABLE_HASHSIZE 211

typedef struct avm_table_bucket avm_table_bucket;
typedef struct avm_table avm_table;

void
execute_newtable(instruction* instr);

void
execute_tablegetelem(instruction* instr);

void
execute_tablesetelem(instruction* instr);

avm_table*
avm_tablenew();

void
avm_tabledestroy(avm_table* t);

avm_memcell*
avm_tablegetelem(avm_table* table, avm_memcell* index);

void
avm_tablesetelem(avm_table* table, avm_memcell* index, avm_memcell* content);

#endif