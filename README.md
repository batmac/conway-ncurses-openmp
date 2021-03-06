# conway-ncurses-openmp
multithreading Conway's Game of Life with openmp, rendering is done with 
libncurses (also libcaca or sdl) 

[Conway's Game of Life](https://en.wikipedia.org/wiki/Conway%27s_Game_of_Life)

## build
have at least ncurses dev files, optionnaly SDL and/or libcaca 's ones.
```
make
```

## options
```
$ ./nlife-ncurses -h
Usage:
-t <ms>   pause <ms> milliseconds between each generation.
-p <n>    probability to have a living cell somewhere is 1/<n>.
-b        bench, doesn't display anything but stats.
-l <file> load <file>.
-g <n>    stop after <n> generations.
-L <n>    <n> lines (only with -b).
-R <n>    <n> rows (only with -b).
```

## Examples
run with rendering, in a random initial world.
```
./nlife-omp-ncurses
```
run the multi-threaded version (with OpenMP defaults,should have N threads with
N being your logical cores count), no rendering, world is 1280x1280,
10000 generations.
```
./nlife-omp-ncurses -L1280 -R1280 -b -g 10000
```
run the single-threaded version, no rendering, world is 1280x1280, 
10000 generations.
```
./nlife-ncurses -L1280 -R1280 -b -g 10000
```
run with 5 OpenMP threads for 10s, no rendering, world is 1280x1280,  init 
the world with data in EXAMPLES/s1.l .
```
OMP_NUM_THREADS=5 timeout -s INT 10 ./nlife-omp-ncurses -l EXAMPLES/s1.l -L1280 -R1280 -b
```


## disable rendering (for benchmarks):
use -b


## notes
comments are in _french_
