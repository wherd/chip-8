CC=clang
CFLAGS=-Wall -Wextra -std=c11 -pedantic -O2
SDLFLAGS=`pkg-config sdl3 --cflags`
LIBS=`pkg-config sdl3 --libs`

c8: src/c8.c
	$(CC) $(CFLAGS) $(SDLFLAGS) -o c8 src/c8.c $(LIBS)

clean:
	rm -rf c8 *.dSYM