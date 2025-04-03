exe: parser_util.o test.o symbol_table.o
	gcc -o exe parser_util.o test.o symbol_table.o

test.o: test.c
	gcc -c test.c -o test.o

symbol_table.o: ../symbol_table/symbol_table.c
	gcc -c ../symbol_table/symbol_table.c -o symbol_table.o

parser_util.o: parser_util.c
	gcc -c parser_util.c -o parser_util.o

clean:
	rm -f *.o exe