
CFLAGS = -Wall -O3
#-ftree-vectorizer-verbose=2
OMP = -fopenmp -DUSE_OMP

all : ncurses
allr : singlethread omp

singlethread: nlife-ncurses nlife-caca nlife-sdl

omp: nlife-omp-ncurses nlife-omp-caca nlife-omp-sdl

ncurses: nlife-ncurses nlife-omp-ncurses

nlife-ncurses : nlife.c
	$(CC) $(CFLAGS) $< -o $@ -lncurses -DUSE_LIBNCURSES -Wno-unknown-pragmas

nlife-caca : nlife.c
	-$(CC) $(CFLAGS) $< -o $@ `caca-config --cflags` `caca-config --libs` -DUSE_LIBCACA -Wno-unknown-pragmas

nlife-sdl : nlife.c
	-$(CC) $(CFLAGS) $< -o $@ `sdl-config --cflags` `sdl-config --libs` -DUSE_SDL -Wno-unknown-pragmas



nlife-omp-ncurses : nlife.c
	$(CC) $(CFLAGS) $(OMP) $< -o $@ -lncurses -DUSE_LIBNCURSES

nlife-omp-caca : nlife.c
	-$(CC) $(CFLAGS) $(OMP) $< -o $@ `caca-config --cflags` `caca-config --libs` -DUSE_LIBCACA

nlife-omp-sdl : nlife.c
	-$(CC) $(CFLAGS) $(OMP) $< -o $@ `sdl-config --cflags` `sdl-config --libs` -DUSE_SDL


clean :
	-rm -f nlife-caca nlife-ncurses nlife-sdl
	-rm -f nlife-omp-caca nlife-omp-ncurses nlife-omp-sdl
