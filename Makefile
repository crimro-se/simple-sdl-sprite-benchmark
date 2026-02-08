OPTIM = -O3

CFLAGS = -std=c99 -pedantic -Wall -Wextra
SDL3_FLAGS = -lSDL3 
SDL2_FLAGS = -I/usr/include/SDL2 -D_GNU_SOURCE=1 -D_REENTRANT -lSDL2 -lSDL2_image

#PSP_DIR = /opt/pspdev
#PSP_FLAGS = -I$(PSP_DIR)/psp/include -L$(PSP_DIR)/psp/lib/ -L$(PSP_DIR)/psp/sdk/lib/ -Wl,-Bstatic -lSDL3 -lSDL3_image -Wl,--no-whole-archive -lc -lGL -lpspvram -lpspvramalloc -lpspvfpu -lpspge -lpspgu -lpspdisplay -lpspaudio -lpsphprm -lpspctrl -lpsppower -ljpeg -lpng16 -lz

all: bench_sdl3 bench_sdl2

bench_sdl3: 
	gcc $(CFLAGS) $(OPTIM) sdl3.c $(SDL3_FLAGS) -o bench_sdl3

bench_sdl2: 
	gcc $(CFLAGS) $(OPTIM) sdl2.c $(SDL2_FLAGS) -o bench_sdl2

clean:
	rm bench_sdl3 bench_sdl2

#psp:
#	$(PSP_DIR)/bin/psp-gcc $(CFLAGS) $(OPTIM) benchmark.c $(PSP_FLAGS) -o benchmark_psp