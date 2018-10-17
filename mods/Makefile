# srv

CC = gcc
CFLAGS = -I../include/ -pipe -O2 -W -Wall -Wno-unused -fPIC

all: mods

mod_test: mod_test.c
	${CC} ${CFLAGS} -c mod_test.c
	${CC} ${CFLAGS} -shared mod_test.o -o mod_test.so

mod_pic: mod_pic.c
	${CC} ${CFLAGS} -c mod_pic.c
	${CC} ${CFLAGS} -shared mod_pic.o -o mod_pic.so

mods: mod_test mod_pic
	mv mod_*.so ../lib/

clean:
	rm -f *.o
