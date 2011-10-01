# srv

all: srv

dirs:
	mkdir -p lib/
	mkdir -p include/util/

mods:
	make -C mods

srv: dirs mods
	make -C mods
	make -C src/util
	make -C test
	make -C src

debug: dirs mods
	make -C mods
	make -C src/util debug
	make -C test
	make -C src debug

clean:
	make -C mods clean
	make -C test clean
	make -C src clean
	rm -f *.o lib/*.so
	rm -f include/util/*.h
