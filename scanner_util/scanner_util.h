#ifndef SCANNER_H
#define SCANNER_H

#include "../comment_reader/comment_reader.h"

typedef struct Alpha_Token_T Alpha_Token_T;

Alpha_Token_T*
scannerUtil_initializeTokenList();

void
scannerUtil_insertKeyword(Alpha_Token_T* head, unsigned int numLine, char* content);

void
scannerUtil_insertPunctuation(Alpha_Token_T* head, unsigned int numLine, char* content);

void
scannerUtil_insertOperator(Alpha_Token_T* head, unsigned int numLine, char* content);

void
scannerUtil_insertInteger(Alpha_Token_T* head, unsigned int numLine, char* content);

void
scannerUtil_insertFloat(Alpha_Token_T* head, unsigned int numLine, char *content);

void
scannerUtil_insertString(Alpha_Token_T* head, unsigned int numLine, char* content);

void
scannerUtil_insertIdentifier(Alpha_Token_T* head, unsigned int numLine, char* content);

void
scannerUtil_insertLineComment(Alpha_Token_T* head, unsigned int numLine);

void
scannerUtil_insertBlockComment(Alpha_Token_T* head, unsigned int numLine, char* content);

void
scannerUtil_initializeCommentReader();

void
scannerUtil_readComment(unsigned int line);

BlockComment_T*
scannerUtil_matchComment(unsigned int line);

unsigned int
scannerUtil_isFinalBlockComment();

void
scannerUtil_freeCommentReader();

void
scannerUtil_initializeStringReader();

void
scannerUtil_readChar(char c);

char*
scannerUtil_getString();

void
scannerUtil_freeStringReader();

void
scannerUtil_printTokenList(Alpha_Token_T* head);

#endif