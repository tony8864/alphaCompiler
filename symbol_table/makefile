all: test.o symbol_table.o 
	gcc -o exe test.o symbol_table.o 

test.o: test.c
	gcc -c test.c -o test.o

symbol_table.o: symbol_table.c
	gcc -c symbol_table.c -o symbol_table.o

clean:
	rm -f exe *.o