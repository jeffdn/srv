# srv

all: mods

mod_test: mod_test.c
	gcc -pipe -O2 -W -Wall -Wno-unused -fPIC -c mod_test.c
	gcc -shared mod_test.o -o mod_test.so 

mod_pic: mod_pic.c
	gcc -pipe -O2 -W -Wall -Wno-unused -fPIC -c mod_pic.c
	gcc -shared mod_pic.o -o mod_pic.so 

mods: mod_test mod_pic
	mv mod_*.so ../lib/

clean:
	rm -f *.o
