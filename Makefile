CC=gcc
CCOPTS=-g -O0 -Wall -Werror -pedantic

all: principal

principal: principal.c
	${CC} ${CCOPTS} procList.c valList.c principal.c -o principal

run: principal
	./principal

clean:
	rm principal

