#ifndef STRING_READER_H
#define STRING_READER_H

typedef struct StringReader_T StringReader_T;

StringReader_T*
stringReader_initializeReader();

void
stringReader_readChar(StringReader_T* reader, char c);

char*
stringReader_getString(StringReader_T* reader);

void
stringReader_printString(StringReader_T* reader);

#endif