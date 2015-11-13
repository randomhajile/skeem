skeem: skeem.o structures.o primitives.o
	cc -g -o skeem skeem.o structures.o primitives.o

skeem.o: skeem.c skeem.h token.h structures.h primitives.h
	cc -g -c skeem.c

structures.o: structures.c structures.h common.h op_codes.h
	cc -g -c structures.c

primitives.o: primitives.c primitives.h structures.h
	cc -g -c primitives.c

clean:
	rm -rf skeem *.o
