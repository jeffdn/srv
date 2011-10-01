# srvtest

CC = gcc
CFLAGS = -I../include/ -pipe -O2 -W -Wno-unused -Wall

all: srvtest

srvtest.o: srvtest.c
	${CC} ${CFLAGS} -c srvtest.c

srvtest: srvtest.o
	${CC} ${CFLAGS} srvtest.o ../src/util/{sock,util}.o -lpthread -o srvtest

clean:
	rm -f *.o srvtest
