# conway-ncurses-openmp
multithreading Conway's Game of Life with openmp, rendering is done with libncurses (also libcaca or sdl) 

[Conway's Game of Life](https://en.wikipedia.org/wiki/Conway%27s_Game_of_Life)

## build
have at least ncurses dev files, optionnaly SDl and/or libcaca 's ones.
`make` 

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

## openmp
```
make omp 
#or
make -f Makefile-omp 

OMP_NUM_THREADS=5 timeout -s INT 10 ./nlife-omp-ncurses -l EXAMPLES/s1.l -L1280 -R1280 -b
```


## bench 
use -b

## cache ??

## notes
comments are in _french_
