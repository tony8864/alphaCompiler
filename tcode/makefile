exe: icode.o symtab.o tcode.o space.o
	gcc -o exe icode.o symtab.o tcode.o space.o

icode.o: ../icode/icode.c
	gcc -c ../icode/icode.c -o icode.o

symtab.o: ../symbol_table/symbol_table.c
	gcc -c ../symbol_table/symbol_table.c -o symtab.o

space.o: ../scope_space/scope_space.c
	gcc -c ../scope_space/scope_space.c -o space.o

tcode.o: tcode.c
	gcc -c tcode.c -o tcode.o

clean:
	rm -f exe *.o