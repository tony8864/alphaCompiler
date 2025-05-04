#include "avm.h"
#include "loader/loader.h"

#include <stdio.h>
#include <stdlib.h>

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

    loader_loadBinaryFile(binaryFile);
    loader_loadCode();

    fclose(binaryFile);

    return 0;
}