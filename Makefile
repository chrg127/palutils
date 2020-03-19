# Simple makefile to create getcolorvals and getpal binaries.

default:
	$(info Please select a target (getcolorvals | getpal))

getcolorvals: getcolorvals.o colorlist.o
	gcc obj/getcolorvals.o obj/colorlist.o -o out/getcolorvals

debuggetcval: getcolorvals_g.o colorlist_g.o
	gcc obj/getcolorvals.o obj/colorlist.o -o out/getcolorvals_debug

getpal: getpal.o readpng.o colorlist.o
	gcc obj/getpal.o obj/readpng.o obj/colorlist.o -lz -lpng -o out/getpal

debuggetpal: getpal_g.o readpng_g.o colorlist_g.o
	gcc obj/getpal.o obj/readpng.o obj/colorlist.o -lz -lpng -o out/getpal_debug

getcolorvals.o: getcolorvals.c
	gcc -c getcolorvals.c -o obj/getcolorvals.o

getcolorvals_g.o: getcolorvals.c
	gcc -g -c getcolorvals.c -o obj/getcolorvals.o

getpal.o: getpal.c
	gcc -c getpal.c -o obj/getpal.o

getpal_g.o: getpal.c
	gcc -g -c getpal.c -o obj/getpal.o

readpng.o: readpng.c
	gcc -c readpng.c -o obj/readpng.o

readpng_g.o: readpng.c
	gcc -g -c readpng.c -o obj/readpng.o

colorlist.o: colorlist.c
	gcc -c colorlist.c -o obj/colorlist.o

colorlist_g.o: colorlist.c
	gcc -g -c colorlist.c -o obj/colorlist.o

clean:
	rm obj/* out/*
