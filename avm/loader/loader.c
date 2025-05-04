#include "loader.h"
#include "../avm.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

FILE* binaryFile = NULL;

double* numConsts = NULL;
unsigned totalNumConsts = 0;

char** stringConsts = NULL;
unsigned totalStringConsts = 0;

char** namedLibFuncs = NULL;
unsigned totalNamedLibFuncs = 0;

userfunc* userFuncs = NULL;
unsigned totalUserFuncs = 0;

/* ------------------------------------ Static Declarations ------------------------------------ */
static void
readMagicNUmber();

static void
loadArrays();

static void
loadStringArray();

static void
loadNumArray();

static void
loadUserFuncs();

static void
loadLibFuncs();

static void
loadInstructions();

static void
printStringArray();

static void
printNumArray();

static void
printUserFuncs();

static void
printLibFuncs();

/* ------------------------------------ Implementation ------------------------------------ */
void
loader_loadBinaryFile(FILE* file) {
    assert(file);
    binaryFile = file;
}

void
loader_loadCode() {
    readMagicNUmber();
    loadArrays();
    loadInstructions();

    printStringArray();
    printNumArray();
    printUserFuncs();
    printLibFuncs();
}

/* ------------------------------------ Static Definitions ------------------------------------ */
static void
readMagicNUmber() {
    int number;
    if (fscanf(binaryFile, "%d", &number) != 1) {
        printf("Error reading magic number from binary file.\n");
        exit(EXIT_FAILURE);
    }

    if (number != 340200501) {
        printf("Error: Magic number is incorrect.\n");
        exit(1);
    }
}

static void
loadArrays() {
    loadStringArray();
    loadNumArray();
    loadUserFuncs();
    loadLibFuncs();
}

static void
loadStringArray() {
    if (fscanf(binaryFile, "%u", &totalStringConsts) != 1) {
        printf("Error reading total string consts from binary file.\n");
        exit(1);
    }

    stringConsts = malloc(sizeof(char*) * totalStringConsts);
    if (!stringConsts) {
        printf("Error allocating memory for string consts array.\n");
        exit(1);
    }

    int len;
    int ch;
    for (int i = 0; i < totalStringConsts; i++) {
        if (fscanf(binaryFile, "%d", &len) != 1) {
            printf("Error reading length of string. %d\n", len);
            exit(1);
        }

        // consume space after strings length
        fgetc(binaryFile);

        stringConsts[i] = malloc(sizeof(char) * (len + 1));
        if (!stringConsts[i]) {
            printf("Error allocating memory for const string.\n");
            exit(1);
        }

        for (int j = 0; j < len; j++) {
            ch = fgetc(binaryFile);
            if (ch == EOF) {
                printf("Error: Unexpected EOF while reading string.\n");
                exit(1);
            }

            stringConsts[i][j] = (char) ch;
        }

        stringConsts[i][len] = '\0';
    }
}

static void
loadNumArray() {
    if (fscanf(binaryFile, "%u", &totalNumConsts) != 1) {
        printf("Error reading total num consts from binary file.\n");
        exit(1);
    }

    numConsts = malloc(sizeof(double) * totalNumConsts);
    if (!numConsts) {
        printf("Error allocating memory for num consts.\n");
        exit(1);
    }

    int len;
    double n;
    for (int i = 0; i < totalNumConsts; i++) {
        if (fscanf(binaryFile, "%lf", &n) != 1) {
            printf("Error reading num const.\n");
            exit(1);
        }

        numConsts[i] = n;
    }
}

static void
loadUserFuncs() {
    if (fscanf(binaryFile, "%u", &totalUserFuncs) != 1) {
        printf("Error reading total user funcs from binary file.\n");
        exit(1);
    }

    userFuncs = malloc(sizeof(userfunc) * totalUserFuncs);
    if (!userFuncs) {
        printf("Error allocating memory for user funcs.\n");
        exit(1);
    }

    userfunc f;
    unsigned address;
    unsigned localSize;
    char* id;
    int len;
    char ch;
    for (int i = 0; i < totalUserFuncs; i++) {
        if (fscanf(binaryFile, "%u", &address) != 1) {
            printf("Error reading address of user function.\n");
            exit(1);
        }

        if (fscanf(binaryFile, "%u", &localSize) != 1) {
            printf("Error reading local size of user function.\n");
            exit(1);
        }

        if (fscanf(binaryFile, "%d", &len) != 1) {
            printf("Error reading length of user func. %d\n", len);
            exit(1);
        }

        // consume space after reading user func len
        fgetc(binaryFile);

        id = malloc(sizeof(char) * (len + 1));
        if (!id) {
            printf("Error allocating memory for user func id.\n");
            exit(1);
        }

        for (int j = 0; j < len; j++) {
            ch = fgetc(binaryFile);
            if (ch == EOF) {
                printf("Error: Unexpected EOF while reading user func.\n");
                exit(1);
            }

            id[j] = (char) ch;
        }
        
        id[len] = '\0';

        userFuncs[i].address = address;
        userFuncs[i].localSize = localSize;
        userFuncs[i].id = id;
    }
}

static void
loadLibFuncs() {
    if (fscanf(binaryFile, "%u", &totalNamedLibFuncs) != 1) {
        printf("Error reading total string consts from binary file.\n");
        exit(1);
    }

    namedLibFuncs = malloc(sizeof(char*) * totalNamedLibFuncs);
    if (!namedLibFuncs) {
        printf("Error allocating memory for string consts array.\n");
        exit(1);
    }

    int len;
    int ch;
    for (int i = 0; i < totalNamedLibFuncs; i++) {
        if (fscanf(binaryFile, "%d", &len) != 1) {
            printf("Error reading length of string. %d\n", len);
            exit(1);
        }

        // consume space after strings length
        fgetc(binaryFile);

        namedLibFuncs[i] = malloc(sizeof(char) * (len + 1));
        if (!namedLibFuncs[i]) {
            printf("Error allocating memory for const string.\n");
            exit(1);
        }

        for (int j = 0; j < len; j++) {
            ch = fgetc(binaryFile);
            if (ch == EOF) {
                printf("Error: Unexpected EOF while reading string.\n");
                exit(1);
            }

            namedLibFuncs[i][j] = (char) ch;
        }

        namedLibFuncs[i][len] = '\0';
    }
}

static void
loadInstructions() {

}

static void
printStringArray() {
    char* str;
    for (int i = 0; i < totalStringConsts; i++) {
        str = stringConsts[i];
        printf("%d: %s\n", i, str);
    }
}

static void
printNumArray() {
    double n;
    for (int i = 0; i < totalNumConsts; i++) {
        n = numConsts[i];
        printf("%d: %f\n", i, numConsts[i]);
    }
}

static void
printUserFuncs() {
    userfunc f;
    for (int i = 0; i < totalUserFuncs; i++) {
        f = userFuncs[i];
        printf("%u %u %s\n", f.address, f.localSize, f.id);
    }
}

static void
printLibFuncs() {
    char* lib;
    for (int i = 0; i < totalNamedLibFuncs; i++) {
        lib = namedLibFuncs[i];
        printf("%d: %s\n", i, lib);
    }
}