CC=gcc
CCOPTS=-g -O0 -Wall -Werror -pedantic
MODULES=procList.o valList.o fs.o

all: principal

modules:
	${CC} ${CCOPTS} -c procList.c valList.c fs.c

principal: modules
	${CC} ${CCOPTS} ${MODULES} principal.c -o principal

run: principal
	./principal

clean:
	rm *.o && rm -f principal

tar:
	tar czvf msh.tar.gz *.c *.h Makefile
