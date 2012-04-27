

cube: cube.c Makefile
	gcc -Wall -g3 -o $@ $< $(shell pkg-config --cflags --libs clutter-1.0)
