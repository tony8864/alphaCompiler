#include "symbol_table.h"

#include <stdio.h>

int main() {
    SymbolTable* table = symtab_initialize();
    SymbolTableEntry* entry;

    symtab_insertFunction(table, "var1", 1, 1, LIBFUNC);
    symtab_insertFunction(table, "var2", 1, 2, LIBFUNC);
    symtab_insertFunction(table, "var3", 1, 3, LIBFUNC);

    symtab_insertFunction(table, "var4a", 1, 4, LIBFUNC);
    symtab_insertFunction(table, "var4b", 1, 4, LIBFUNC);
    symtab_insertFunction(table, "var4c", 1, 4, LIBFUNC);
    symtab_insertFunction(table, "var4d", 1, 4, LIBFUNC);


    //symtab_printCollisionTable(table);
    symtab_printScopeTable(table);


    symtab_hide(table, 4);
    symtab_hide(table, 1);

    symtab_printScopeTable(table);


    return 0;
}