SRCDIR=../src

all : parse.o pmake.o run_make.o

parse.o : ${SRCDIR}/parse.c
	gcc -Wall -g -c ${SRCDIR}/parse.c

pmake.o : ${SRCDIR}/pmake.c
	gcc -Wall -g -c ${SRCDIR}/pmake.c

run_make.o : ${SRCDIR}/run_make.c
	gcc -Wall -g -c ${SRCDIR}/run_make.c

clean :
	rm *.o