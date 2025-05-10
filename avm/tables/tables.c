#include "tables.h"

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

typedef struct avm_table_bucket {
    avm_memcell key;
    avm_memcell value;
    avm_table_bucket* next;
} avm_table_bucket;

typedef struct avm_table {
    unsigned refCounter;
    avm_table_bucket* strIndexed[AVM_TABLE_HASHSIZE];
    avm_table_bucket* numIndexed[AVM_TABLE_HASHSIZE];
    unsigned totalStrIndexed;
    unsigned totalNumIndexed;
} avm_table;

/* ---------------------------------- Static Declarations ---------------------------------- */
static void
avm_tableIncrementRefCounter(avm_table* t);

static void
avm_tableDecrementRefCounter(avm_table* t);

static void
avm_tablebucketsinit(avm_table_bucket** p);

static void
avm_tablebucketsdestroy(avm_table_bucket** p);

static unsigned
hash_string(const char* str);

/* ---------------------------------- Implementation ---------------------------------- */
void
execute_newtable(instruction* instr) {
    avm_memcell* lv = avm_translate_operand(&instr->arg1, NULL);
    assert(lv && (&stack[top] < lv && lv <= &stack[N - 1]) || lv == &retval);

    avm_memcellclear(lv);

    lv->type = table_m;
    lv->data.tableVal = avm_tablenew();
    avm_tableIncrementRefCounter(lv->data.tableVal);
}

void
execute_tablegetelem(instruction* instr) {
    avm_memcell* lv = avm_translate_operand(&instr->result, NULL);
    avm_memcell* t = avm_translate_operand(&instr->arg1, NULL);
    avm_memcell* i = avm_translate_operand(&instr->arg2, &ax);

    assert(lv && (lv > &stack[top] && lv <= &stack[N - 1]) || lv == &retval);
    assert(t && &stack[N - 1] >= t && t > &stack[top]);
    assert(i);

    avm_memcellclear(lv);
    lv->type = nil_m;

    if (t->type != table_m) {
        printf("Illegal use of type type as table.\n");
        exit(1);
    }

    avm_memcell* content = avm_tablegetelem(t->data.tableVal, i);

    if (content) {
        avm_assign(lv, content);
    }
    else {
        printf("Key not found.\n");
        exit(1);
    }
}

void
execute_tablesetelem(instruction* instr) {
    avm_memcell* t = avm_translate_operand(&instr->arg1, NULL);
    avm_memcell* i = avm_translate_operand(&instr->arg2, &ax);
    avm_memcell* c = avm_translate_operand(&instr->result, &bx);

    assert(t && &stack[N - 1] >= t && t > &stack[top]);
    assert(i && c);

    if (t->type != table_m) {
        printf("illegal use of type as table.\n");
        exit(1);
    }

    avm_tablesetelem(t->data.tableVal, i, c);
}

avm_table*
avm_tablenew() {
    avm_table* t = malloc(sizeof(avm_table));
    memset(t, 0, sizeof(t));
    t->refCounter = 0;
    t->totalNumIndexed = 0;
    t->totalStrIndexed = 0;
    avm_tablebucketsinit(t->numIndexed);
    avm_tablebucketsinit(t->strIndexed);
    return t;
}

void
avm_tabledestroy(avm_table* t) {
    avm_tablebucketsdestroy(t->strIndexed);
    avm_tablebucketsdestroy(t->numIndexed);
    free(t);
}

avm_memcell*
avm_tablegetelem(avm_table* table, avm_memcell* index) {
    avm_table_bucket* curr;
    avm_table_bucket* prev;

    switch (index->type) {
        case string_m: {
            unsigned int hash = hash_string(index->data.strVal);
            curr = table->strIndexed[hash];

            while (curr) {
                prev = curr;
                if (strcmp(curr->key.data.strVal, index->data.strVal) == 0) {
                    return &(curr->value);
                }
                curr = curr->next;
            }
            break;
        }
        case number_m: {

        }
        default: {
            printf("table index type not supported.\n");
            exit(1);
        }
    }
}

void
avm_tablesetelem(avm_table* table, avm_memcell* index, avm_memcell* content) {

    avm_table_bucket* curr;
    avm_table_bucket* prev;

    switch (index->type) {
        case string_m: {
            unsigned hash = hash_string(index->data.strVal);
            curr = table->strIndexed[hash];
            prev = NULL;

            while (curr) {
                prev = curr;
                if (strcmp(curr->key.data.strVal, index->data.strVal) == 0) {
                    curr->value = *content;
                    return;
                }
                curr = curr->next;
            }
            
            avm_table_bucket* node;

            node = malloc(sizeof(avm_table_bucket));

            if (!node) {
                printf("Error allocating memory for new bucket node.\n");
                exit(1);
            }

            node->next = NULL;
            node->key = *index;
            node->value = *content;

            if (content->type == string_m) {
                node->value.data.strVal = strdup(content->data.strVal);
            }
            if (index->type == string_m) {
                node->key.data.strVal = strdup(index->data.strVal);
            }

            if (prev == NULL) {
                table->strIndexed[hash] = node;
            }
            else {
                prev->next = node;
            }
            break;
        }
        case number_m: {

        }
        default: {
            printf("table index type not supported.\n");
            exit(1);
        }
    }
}

/* ---------------------------------- Static Definitions ---------------------------------- */
static void
avm_tableIncrementRefCounter(avm_table* t) {
    ++t->refCounter;
}

static void
avm_tableDecrementRefCounter(avm_table* t) {
    assert(t->refCounter > 0);
    if (!--t->refCounter) {
        avm_tabledestroy(t);
    }
}

static void
avm_tablebucketsinit(avm_table_bucket** p) {
    for (unsigned i = 0; i < AVM_TABLE_HASHSIZE; i++) {
        p[i] = NULL;
    }
}

static void
avm_tablebucketsdestroy(avm_table_bucket** p) {
    for (unsigned i = 0; i < AVM_TABLE_HASHSIZE; i++) {
        for (avm_table_bucket* b = *p; b;) {
            avm_table_bucket* del = b;
            b = b->next;
            avm_memcellclear(&del->key);
            avm_memcellclear(&del->value);
            free(del);
        }
        p[i] = NULL;
    }
}

static unsigned
hash_string(const char* str) {
    unsigned long hash = 5381;
    int c;

    while ((c = *str++)) {
        hash = ((hash << 5) + hash) + c;  // hash * 33 + c
    }

    return hash % AVM_TABLE_HASHSIZE;
}
