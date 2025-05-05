#include "avm.h"
#include "dispatcher/dispatcher.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char** argv) {

    FILE* binaryFile;

    if (argc < 2) {
        printf("Provide the path to the binary file.\n");
        exit(1);
    }

    binaryFile = fopen(argv[1], "r");

    if (!binaryFile) {
        printf("Error opening binary file.\n");
        exit(1);
    }

    dispatcher_initialize(binaryFile);
    dispatcher_execute_cycle();

    fclose(binaryFile);

    return 0;
}