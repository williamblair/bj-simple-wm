# makefile for bj-simple-wm

all:
	gcc main.c -o bj-simple-wm -lX11

clean:
	rm -f bj-simple-wm

