tims: tims.o structures.o primitives.o
	cc -g -o tims tims.o structures.o primitives.o

tims.o: tims.c tims.h token.h structures.h primitives.h
	cc -g -c tims.c

structures.o: structures.c structures.h common.h op_codes.h
	cc -g -c structures.c

primitives.o: primitives.c primitives.h structures.h
	cc -g -c primitives.c

clean:
	rm -rf tims *.o
