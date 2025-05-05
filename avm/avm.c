#include <stdio.h>
#include <stdlib.h>

#include "loader/loader.h"

int main(int argc, char** argv) {

    if (argc <= 1) {
        printf("You must provide the path of the binary file.\n");
        exit(1);
    }

    loader_openBinaryFile(argv[1]);

    loader_load_avm_constants();

    return 0;
}