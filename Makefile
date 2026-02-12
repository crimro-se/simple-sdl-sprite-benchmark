OPTIM = -O3

CFLAGS = -std=c99 -pedantic -Wall -Wextra
SDL3_FLAGS = -DSDL3 -lSDL3
SDL2_FLAGS = -I/usr/include/SDL2 -D_GNU_SOURCE=1 -D_REENTRANT -lSDL2 -lSDL2_image

all: bench

bench:
	gcc $(CFLAGS) $(OPTIM) bench.c $(SDL2_FLAGS) -o bench_sdl2
	gcc $(CFLAGS) $(OPTIM) bench.c $(SDL3_FLAGS) -o bench_sdl3

clean:
	rm bench_sdl2 bench_sdl3