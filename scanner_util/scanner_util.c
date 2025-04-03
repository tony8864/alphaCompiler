#include "scanner_util.h"
#include "../string_reader/string_reader.h"
#include "../comment_reader/comment_reader.h"

#include <stdio.h>
#include <stdlib.h>

typedef struct Alpha_Token_T {
    unsigned int numLine;
    unsigned int numToken;
    char* content;
    char* type;
    Alpha_Token_T* next;
} Alpha_Token_T;

StringReader_T*     stringReader    = NULL;
CommentReader_T*    commentReader   = NULL;

static Alpha_Token_T*
initializeToken(unsigned int numLine, char* content, char* type);

static void
insertToken(Alpha_Token_T* head, unsigned int numLine, char* content, char* type);

Alpha_Token_T*
scannerUtil_initializeTokenList() {
    Alpha_Token_T* head;

    head = malloc(sizeof(Alpha_Token_T));

    if (!head) {
        printf("Failed to initialize alpha token list.\n");
        exit(1);
    }

    head->numLine = 0;
    head->numToken = 0;
    head->content = NULL;
    head-> type = NULL;
    head->next = NULL;

    return head;
}

void
scannerUtil_insertKeyword(Alpha_Token_T* head, unsigned int numLine, char* content) {
    insertToken(head, numLine, content, "KEYWORD");
}

void
scannerUtil_insertPunctuation(Alpha_Token_T* head, unsigned int numLine, char* content) {
    insertToken(head, numLine, content, "PUNCTUATION");
}

void
scannerUtil_insertOperator(Alpha_Token_T* head, unsigned int numLine, char* content) {
    insertToken(head, numLine, content, "OPERATOR");
}

void
scannerUtil_insertInteger(Alpha_Token_T* head, unsigned int numLine, char* content) {
    insertToken(head, numLine, content, "INTCONST");
}

void
scannerUtil_insertFloat(Alpha_Token_T* head, unsigned int numLine, char *content) {
    insertToken(head, numLine, content, "REALCONST");
}

void
scannerUtil_insertString(Alpha_Token_T* head, unsigned int numLine, char* content) {
    insertToken(head, numLine, content, "STRING");
}

void
scannerUtil_insertIdentifier(Alpha_Token_T* head, unsigned int numLine, char* content) {
    insertToken(head, numLine, content, "IDENTIFIER");
}

void
scannerUtil_insertLineComment(Alpha_Token_T* head, unsigned int numLine) {
    insertToken(head, numLine, "", "LINE_COMMENT");
}

void
scannerUtil_insertBlockComment(Alpha_Token_T* head, unsigned int numLine, char* content) {
    insertToken(head, numLine, content, "BLOCK_COMMENT");
}

void
scannerUtil_initializeCommentReader() {
    commentReader = commentReader_initializeReader();
}

void
scannerUtil_readComment(unsigned int line) {
    if (commentReader == NULL) { 
        printf("Comment reader is not initialized.\n");
        exit(1);
    }

    commentReader_readComment(commentReader, line);
}

BlockComment_T*
scannerUtil_matchComment(unsigned int line) {
    if (commentReader == NULL) { 
        printf("Comment reader is not initialized.\n");
        exit(1);
    }

    return commentReader_matchComment(commentReader, line);
}

unsigned int
scannerUtil_isFinalBlockComment() {
    return commentReader_getNumberOfComments(commentReader) == 0;
}

void
scannerUtil_freeCommentReader() {
    if (commentReader == NULL) {
        printf("Comment reader is not initialized.\n");
        exit(1);
    }

    free(commentReader);
}

void
scannerUtil_initializeStringReader() {
    stringReader = stringReader_initializeReader();
}

void
scannerUtil_readChar(char c) {
    if (stringReader == NULL) { 
        printf("String reader is not initialized.\n");
        exit(1);
    }

    stringReader_readChar(stringReader, c);
}

char*
scannerUtil_getString() {
    if (stringReader == NULL) { 
        printf("String reader is not initialized.\n");
        exit(1);
    }

    return stringReader_getString(stringReader);
}

void
scannerUtil_freeStringReader() {
    if (stringReader == NULL) { 
        printf("String reader is not initialized.\n");
        exit(1);
    }

    free(stringReader);
}

void
scannerUtil_printTokenList(Alpha_Token_T* head) {
    Alpha_Token_T* token;

    token = head->next;

    printf("Lexical Analysis\n");
    printf("-----------------------------------------------------------------------\n");
    printf("%-10s %-10s %-25s %-25s\n", "Line no.", "Token no.", "Content", "Type");
    while (token) {

        printf("%-10d %-10d %-25s %-25s\n", token->numLine, token->numToken, token->content, token->type);

        token = token->next;
    }
}

static void
insertToken(Alpha_Token_T* head, unsigned int numLine, char* content, char* type) {
    Alpha_Token_T* tail;
    Alpha_Token_T* token;

    tail = head;
    while (tail->next) {
        tail = tail->next;
    }
    
    token = initializeToken(numLine, content, type);
    
    tail->next = token;
}

static Alpha_Token_T*
initializeToken(unsigned int numLine, char* content, char* type) {

    static unsigned int numToken = 1;

    Alpha_Token_T* token;

    token = malloc(sizeof(Alpha_Token_T));

    if (!token) {
        printf("Failed to initialize token.\n");
        exit(1);
    }

    token->numLine = numLine;
    token->numToken = numToken++;
    token->content = content;
    token->type = type;
    token->next = NULL;

    return token;
}






















