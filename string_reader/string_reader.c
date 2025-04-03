#include "string_reader.h"

#include <stdio.h>
#include <stdlib.h>

#define MAX_SIZE 256

typedef struct StringReader_T {
    char string[MAX_SIZE];
    unsigned int length;
} StringReader_T;

StringReader_T*
stringReader_initializeReader() {
    StringReader_T* reader;

    reader = malloc(sizeof(StringReader_T));

    if (!reader) {
        printf("Error allocating memory for string reader.\n");
        exit(1);
    }

    for (unsigned int i = 0; i < MAX_SIZE; i++) {
        reader->string[i] = '\0';
    }
    reader->length = 0;

    return reader;
}

void
stringReader_readChar(StringReader_T* reader, char c) {
    reader->string[reader->length++] = c;
}

char*
stringReader_getString(StringReader_T* reader) {
    return reader->string;
}

void
stringReader_printString(StringReader_T* reader) {
    printf("String: %s\n", reader->string);
}