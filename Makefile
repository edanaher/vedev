all: vedve

vedve: vedve.c uinput.c
	gcc -O2 -Wall -Werror uinput.c vedve.c -ovedve

install:
	mkdir $(out)/bin
	cp vedve $(out)/bin
