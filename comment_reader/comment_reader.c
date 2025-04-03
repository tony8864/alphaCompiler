#include "comment_reader.h"

#include <stdio.h>
#include <stdlib.h>

#define MAX_NEST 100
#define MAX_SIZE 100

typedef struct CommentReader_T {
    unsigned int commentLineArray[MAX_NEST];
    unsigned int length;
} CommentReader_T;

typedef struct BlockComment_T {
    unsigned int startLine;
    unsigned int endLine;
    char scope[MAX_SIZE];
} BlockComment_T;

static BlockComment_T*
initializeBlockComment(unsigned int startLine, unsigned int endLine);

CommentReader_T*
commentReader_initializeReader() {
    CommentReader_T* reader;

    reader = malloc(sizeof(CommentReader_T));

    if (!reader) {
        printf("Error allocating memory for block comment reader.\n");
        exit(1);
    }

    for (unsigned int i = 0; i < MAX_NEST; i++) {
        reader->commentLineArray[i] = 0;
    }
    reader->length = 0;

    return reader;
}

unsigned int
commentReader_getNumberOfComments(CommentReader_T* reader) {
    return reader->length;
}

void
commentReader_readComment(CommentReader_T* reader, unsigned int line) {
    reader->commentLineArray[reader->length++] = line;
}

BlockComment_T*
commentReader_matchComment(CommentReader_T* reader, unsigned int endLine) {
    BlockComment_T* comment;
    unsigned int startLine;

    startLine = reader->commentLineArray[--reader->length];
    comment = initializeBlockComment(startLine, endLine);

    return comment;
}

unsigned int
commentReader_getCommentStart(BlockComment_T* comment) {
    return comment->startLine;
}

unsigned int
commentReader_getCommentEnd(BlockComment_T* comment) {
    return comment->endLine;
}

char*
commentReader_getCommentScope(BlockComment_T* comment) {
    return comment->scope;
}

void
commentReader_printCommentLines(CommentReader_T* reader) {
    printf("comment line array: ");
    for (unsigned int i = 0; i < reader->length; i++) {
        printf("%d ", reader->commentLineArray[i]);
    }
    printf("\n");
}

void
commentReader_printBlockComment(BlockComment_T* comment) {
    printf("Block comment: %d - %d ... %s\n", comment->startLine, comment->endLine, comment->scope);
}

static BlockComment_T*
initializeBlockComment(unsigned int startLine, unsigned int endLine) {
    BlockComment_T* comment;

    comment = malloc(sizeof(BlockComment_T));

    if (!comment) {
        printf("Error allocating memory for block comment.\n");
        exit(1);
    }

    comment->startLine = startLine;
    comment->endLine = endLine;

    for (unsigned int i = 0; i < MAX_SIZE; i++) {
        comment->scope[i] = '\0';
    }

    sprintf(comment->scope, "%d-%d", startLine, endLine);

    return comment;
}