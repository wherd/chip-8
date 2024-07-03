CC=clang
CFLAGS=-Wall -Wextra -std=c11 -pedantic -g
SDLFLAGS=`pkg-config sdl3 --cflags`
LIBS=`pkg-config sdl3 --libs`

c8: src/c8.c
	$(CC) $(CFLAGS) $(SDLFLAGS) -o c8 src/c8.c $(LIBS)

d8: src/d8.c
	$(CC) $(CFLAGS) -o d8 src/d8.c

clean:
	rm -rf c8 d8 *.dSYM *.ct8