CC=clang
CFLAGS=-Wall -Wextra -std=c11 -pedantic -g `pkg-config sdl3 --cflags`
LIBS=`pkg-config sdl3 --libs`

c8: src/c8.c
	$(CC) $(CFLAGS) -o c8 src/c8.c $(LIBS)
