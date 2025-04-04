
OBJECTS = scanner.o scanner_util.o string_reader.o comment_reader.o parser.o parser_util.o symbol_table.o scope_stack.o

SCOPE_STACK_C = scope_stack/scope_stack.c
SYMBOL_TABLE_C = symbol_table/symbol_table.c
SCANNER_UTIL_C = scanner_util/scanner_util.c
PARSER_UTIL_C = parser_util/parser_util.c
STRING_READER_C = string_reader/string_reader.c
COMMENT_READER_C = comment_reader/comment_reader.c
PARSER_C_H = parser.tab.c parser.tab.h

exe: ${OBJECTS}
	gcc -o exe ${OBJECTS}

scanner.o: scanner.c
	gcc -c $< -o $@

scanner.c: scanner.l parser.tab.h
	flex --outfile=scanner.c $<

parser.o: parser.tab.c
	gcc -c $< -o $@

${PARSER_C_H}: parser.y
	bison -d $<

scanner_util.o:	${SCANNER_UTIL_C}
	gcc -c $< -o $@

string_reader.o: ${STRING_READER_C}
	gcc -c $< -o $@

comment_reader.o: ${COMMENT_READER_C}
	gcc -c $< -o $@
	
parser_util.o: ${PARSER_UTIL_C}
	gcc -c $< -o $@

symbol_table.o: ${SYMBOL_TABLE_C}
	gcc -c $< -o $@

scope_stack.o: ${SCOPE_STACK_C}
	gcc -c $< -o $@

clean:
	rm -f exe scanner.c ${OBJECTS} ${PARSER_C_H}