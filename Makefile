

all: srv

dirs:
	if [ ! -d lib/ ]; then \
	    mkdir -p lib/;     \
    fi
	if [ ! -d include/ ]; then        \
	    mkdir -p include/{srv,util}/; \
	fi
	cp src/*.h include/srv/
	cp src/util/*.h include/util/

mods:
	make -C mods

srv: dirs mods
	make -C src/util
	make -C test
	make -C src
	make -C mods

debug: dirs mods
	make -C src/util debug
	make -C test
	make -C src debug
	make -C mods

clean:
	make -C mods clean
	make -C test clean
	make -C src clean
	rm -f *.o lib/mod_*.so
	rm -f include/{srv,util}/*.h
