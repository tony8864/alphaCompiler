#ifndef COMMENT_READER_H
#define COMMENT_READER_H

typedef struct CommentReader_T CommentReader_T;
typedef struct BlockComment_T BlockComment_T;

CommentReader_T*
commentReader_initializeReader();

unsigned int
commentReader_getNumberOfComments(CommentReader_T* reader);

void
commentReader_readComment(CommentReader_T* reader, unsigned int line);

BlockComment_T*
commentReader_matchComment(CommentReader_T* reader, unsigned int line);

unsigned int
commentReader_getCommentStart(BlockComment_T* comment);

unsigned int
commentReader_getCommentEnd(BlockComment_T* comment);

char*
commentReader_getCommentScope(BlockComment_T* comment);

void
commentReader_printCommentLines(CommentReader_T* reader);

void
commentReader_printBlockComment(BlockComment_T* comment);

#endif