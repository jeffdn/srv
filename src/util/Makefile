# libutil

CC = gcc
CFLAGS = -pipe -O2 -W -Wall -Wno-unused -fPIC

OBJ = sock.o \
	  hash.o \
	  iter.o \
	  util.o \
	  stack.o \
	  module.o \
	  vector.o \
	  thread.o \
	  utstring.o

debug: all

all: libutil

sock.o: sock.h sock.c
	${CC} ${CFLAGS} -c sock.c

module.o: module.h module.c
	${CC} ${CFLAGS} -c module.c

hash.o: hash.h hash.c
	${CC} ${CFLAGS} -c hash.c

iter.o: iter.h iter.c
	${CC} ${CFLAGS} -c iter.c

util.o: util.h util.c
	${CC} ${CFLAGS} -c util.c

stack.o: stack.h stack.c
	${CC} ${CFLAGS} -c stack.c

vector.o: vector.h vector.c
	${CC} ${CFLAGS} -c vector.c

thread.o: thread.h thread.c
	${CC} ${CFLAGS} -c thread.c

utstring.o: utstring.h utstring.c
	${CC} ${CFLAGS} -c utstring.c

openbsd: sock.o stack.o module.o hash.o iter.o util.o vector.o thread.o utstring.o
	${CC} ${CFLAGS} -shared ${OBJ} -o libutil.so
	cp libutil.so ../../lib/
	cp *.h ../../include/util/

osx: sock.o stack.o module.o hash.o iter.o util.o vector.o thread.o utstring.o
	${CC} ${CFLAGS} -dynamic -lpthread ${OBJ} -o libutil.dylib
	cp libutil.dylib ../../lib/
	cp *.h ../../include/util/

libutil: sock.o stack.o module.o hash.o iter.o util.o vector.o thread.o utstring.o
	${CC} ${CFLAGS} -shared -lpthread -ldl ${OBJ} -o libutil.so
	cp libutil.so ../../lib/
	cp *.h ../../include/util/

clean:
	rm -f *.o

remove:
	rm -f *.o libutil.*
