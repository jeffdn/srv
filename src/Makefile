# srv

CC = gcc
CFLAGS = -I../include/ -pipe -O2 -W -Wall -Wno-unused

OBJ = req.o \
	  conn.o \
	  resp.o \
	  conf.o \
	  srv.o

UTIL = hash.o \
	   stack.o \
       thread.o \
	   vector.o \
       utstring.o \
	   util.o

all: srv

util:
	make -C util osx

req.o: req.h req.c
	${CC} ${CFLAGS} -c req.c

conn.o: conn.h conn.c
	${CC} ${CFLAGS} -c conn.c

resp.o: resp.h resp.c
	${CC} ${CFLAGS} -c resp.c

conf.o: conf.h conf.c
	${CC} ${CFLAGS} -c conf.c

srv.o: srv.c
	${CC} ${CFLAGS} -c srv.c

srv: util req.o conn.o resp.o conf.o srv.o
	cp util/{hash,stack,thread,vector,utstring,util}.o .
	${CC} ${CFLAGS} ${OBJ} ${UTIL} -ldl -levent -lpthread -o srv
	mv srv ../
	cp mod.h ../include/srv/

clean:
	make -C util clean
	rm -f *.o ../srv
