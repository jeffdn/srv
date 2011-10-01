# srv

all: srv

dirs:
	mkdir -p lib/
	mkdir -p include/{srv,util}/
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
	rm -f *.o lib/*.so
	rm -f include/{srv,util}/*.h
