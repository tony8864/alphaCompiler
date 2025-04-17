exe: test.o quad.o
	gcc test.o quad.o -o exe

test.o: test.c
	gcc -c test.c -o test.o

quad.o: quad.c
	gcc -c quad.c -o quad.o

clean:
	rm -f *.o exe