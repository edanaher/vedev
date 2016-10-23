all: vedve

vedve: vedve.c uinput.c evdev-capture.c
	gcc -O2 -Wall -Werror -levdev uinput.c evdev-capture.c vedve.c -ovedve

install:
	mkdir $(out)/bin
	cp vedve $(out)/bin
