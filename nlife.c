/* lance un jeu de la vie sur la totalité de la taille de la console */
/* devrait etre plus rapide que l algo naïf */
/*  gcc -Wall -O3 nlife.c -lncurses */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>
#include <string.h>
#ifdef USE_LIBNCURSES
# include <ncurses.h>
#endif
#ifdef USE_LIBCACA
# include <caca.h>
/* libcaca - 0.x API */
# ifdef CACA_API_VERSION_1
#  include <caca0.h>
# endif
#endif
#ifdef USE_SDL
# include <SDL.h>
#endif
#ifdef USE_OMP
#include <omp.h>
#endif

/* lifegame */
time_t start_time;       /* pour afficher des stats */
int nobench = 1;
unsigned long long nbgenerations; /* pour afficher des stats */
unsigned long long nbgenerationsmax = 0; /* pour afficher des stats */
unsigned int lines=100, rows=100;         /* pour afficher des stats dans le finish */
unsigned int *world1, *world2;    /* pour pouvoir libérer la mémoire dans le finish */
unsigned int prob = 5; /* probabilité d'avoir une cellule lors de la creation du monde */
char * filename = NULL;
int init_rand_world(unsigned int* world, unsigned int tlines, unsigned int trows);
int load_file(char * file, unsigned int* world, unsigned int tlines, unsigned int trows);
void finish(int);
/* affichage */
void aff_init(unsigned int * tlines, unsigned int * trows);
void aff_refresh(unsigned int *world , unsigned int tlines, unsigned int trows);
void aff_finish();
/* analyse ligne de commande */
extern char *optarg;

int main(int argc, char * argv[]) {
	register unsigned int i,j;
	char c;
	long pausetime = 0;

	/* analyse de la ligne de commande */
	while((c = (char)getopt (argc, argv, "hbt:p:l:g:L:R:")) != (char)EOF)
		switch(c) {
			case 'h': printf("Usage:\n"
						  "-t <ms>   pause <ms> milliseconds between each generation.\n"
						  "-p <n>    probability to have a living cell somewhere is 1/<n>.\n"
						  "-b        bench, doesn't display anything but stats.\n"
						  "-l <file> load <file>.\n"
						  "-g <n>    stop after <n> generations.\n"
						  "-L <n>    <n> lines (only with -b).\n"
						  "-R <n>    <n> rows (only with -b).\n"
					);
				  exit(0); break;
			case 'b': nobench = 0; break;
			case 't': pausetime = atol(optarg)*1000; break;
			case 'p': prob = atoi(optarg); break;
			case 'l': filename = strdup(optarg); break;
			case 'g': nbgenerationsmax = atoll(optarg); break;
			case 'L': lines = atoll(optarg); break;
			case 'R': rows = atoll(optarg); break;
		}

	if (!nobench) printf("benchmark in progress, Ctrl-C to abort...\n");

	/* on arrete la simulation par un Ctrl-C */
	signal(SIGINT, finish);
	signal(SIGTERM, finish);

	/* init affichage */
	if (nobench) aff_init(&lines, &rows);


	world1 = calloc(sizeof(int), lines*rows);/* init à 0 */
	world2 = calloc(sizeof(int), lines*rows);/* init à 0 */

	/* init du monde */
	if (filename == NULL)
		init_rand_world(world1, lines, rows);
	else
		load_file(filename, world1, lines, rows);

	start_time = time(NULL);
	nbgenerations = 0;

	while(1) {
		if (nbgenerationsmax && nbgenerations >= nbgenerationsmax) finish(0);
		nbgenerations++;
		/* affichage */
		if (nobench)    aff_refresh(world1, lines, rows);
		if (pausetime)  usleep(pausetime);

		/* next gen */
		/* calcul nombre de voisin pour chaque case */
		/* on obtient un tableau du monde avec sur chaque case son nbre de voisins */
#pragma omp parallel for private(j) schedule(dynamic,1)
		/* Warning: obvious race condition may occurs in here... */
		for(i=0;i<lines;++i)
			for(j=0;j<rows;++j) {
				if (world1[i*rows+j] == 1) {
					/* le monde est un tore, attention aux modulos négatifs */
#pragma omp atomic
					world2[(i-1+lines)%lines *rows + (j-1+rows)%rows]++;
#pragma omp atomic
					world2[(i-1+lines)%lines *rows + (j  +rows)%rows]++;
#pragma omp atomic
					world2[(i-1+lines)%lines *rows + (j+1+rows)%rows]++;
#pragma omp atomic
					world2[(i  +lines)%lines *rows + (j-1+rows)%rows]++;
#pragma omp atomic
					world2[(i  +lines)%lines *rows + (j+1+rows)%rows]++;
#pragma omp atomic
					world2[(i+1+lines)%lines *rows + (j-1+rows)%rows]++;
#pragma omp atomic
					world2[(i+1+lines)%lines *rows + (j  +rows)%rows]++;
#pragma omp atomic
					world2[(i+1+lines)%lines *rows + (j+1+rows)%rows]++;
				}
			}
		/* actualisation du tableau suivant le nbre de voisins */
#pragma omp parallel for private(j) schedule(dynamic,1) 
		for(i=0;i<lines;++i)
			for(j=0;j<rows;++j) {
				world1[i*rows+j] =
					((world1[i*rows+j] == 0 && world2[i*rows+j]==3)
					 ||(world1[i*rows+j] == 1 && ( world2[i*rows+j]==3 ||  world2[i*rows+j]==2)))
					?1:0;
				world2[i*rows+j] =0; /* reinit à 0 pour la generation suivante */
			}
	}
}

/********************************************************************/
int init_rand_world(unsigned int* world, unsigned int tlines, unsigned int trows) {
	unsigned int i,j;
	/* init du monde */
	srand(getpid());
#pragma omp parallel for private(j) schedule(dynamic,1) 
	for(i=0;i<tlines;++i)
		for(j=0;j<trows;++j) {
			world[i*trows+j] = (rand()%prob)?0:1 ;
		}
	return 1;
}

/********************************************************************/
int load_file(char * file, unsigned int* world, unsigned int tlines, unsigned int trows) {
	/* parsage bien crado */
	FILE* stream;
	int l = tlines/3, r= trows/3;
	char c;
	int type = 'P';

	if ( (stream = fopen(file, "r")) == NULL)
		return 1;

	while ((c = (char)fgetc(stream)) != (char)EOF) {
		switch (c) {
			case '\n': /* ligne vide */
				break;
			case '#':
				if ((type = fgetc(stream)) != '\n')
					/* passage à la ligne suivante */
					while ((c = (char)fgetc(stream)) != '\n' && c != (char)EOF);
				break;
			default:
				do { /* chargement */
					if (type == 'P') {
						if (c == '.')
							++r;
						else if (c == '\n')
							++l, r = trows/3;
						else if (c == '*' || c == '1')
							++r, world[(l+tlines)%tlines*trows+(r+trows)%trows] = 1;
					}
				} while ((c = (char)fgetc(stream)) != (char)EOF) ;
		}
	}

	fclose(stream);
	printf("import OK\n");
	return 0;
}

/********************************************************************/
void finish(int i) {
	time_t duration;
	if (nobench) aff_finish();
	duration = time(NULL)-start_time;
	if (duration == 0) duration=1;
	printf("We are done.\n");
	printf("World size: %d x %d\n", lines, rows);
	printf("%lld generations, %d seconds (%lld gen/sec, ~ %lld cell/sec)\n", 
			nbgenerations, (int)duration,
			nbgenerations/(long long)duration, nbgenerations*lines*rows/(long long)duration);
	free(world1);
	free(world2);
	if (filename != NULL) free(filename);
	exit(0);
}

/********************************************************************/
/**************************** affichage *****************************/
#ifdef USE_LIBNCURSES

void aff_init(unsigned int * tlines, unsigned int * trows) {
	initscr();      /* initialize the curses library */
	//    nonl();         /* tell curses not to do NL->CR/NL on output */
	if (has_colors()) {
		start_color();
		init_pair(1, COLOR_WHITE, COLOR_BLACK);
		init_pair(2, COLOR_MAGENTA, COLOR_RED);
	}
	/* init taille du monde */
	*tlines = LINES; /* initialisé par ncurses */
	*trows  = COLS;  /* initialisé par ncurses */
	/*     refresh(); */
}

/********************************************************************/
void aff_refresh(unsigned int * world, unsigned int tlines, unsigned int trows) {
	register unsigned int i,j;

	for(i=0;i<tlines;++i){
		for(j=0;j<trows;++j) {
			if (world[i*trows+j] == 1){
				attrset(COLOR_PAIR(2)|WA_BOLD);
				mvaddch(i,j,'@');
			}
			if (world[i*trows+j] == 0) {
				attrset(COLOR_PAIR(1)|WA_NORMAL);
				mvaddch(i,j,' ');
			}
		}
	}
	refresh();
}

/********************************************************************/
void aff_finish() {
	endwin();
}

#endif

#ifdef USE_LIBCACA
/**************************** affichage *****************************/

void aff_init(unsigned int * tlines, unsigned int * trows) {
	if(caca_init() < 0)
		printf("Unable to initialize cacalib\n");
	caca_set_window_title("nlife");
	//caca_set_delay(20000);/* FIXME */
	caca_set_feature(CACA_BACKGROUND_BLACK);
	*tlines = caca_get_height();
	*trows  = caca_get_width();
}

/********************************************************************/
void aff_refresh(unsigned int * world, unsigned int tlines, unsigned int trows) {
	register unsigned int i,j;

	/* exit on any key */
	if (caca_get_event(CACA_EVENT_KEY_PRESS))
		finish(-5);

	for(i=0;i<tlines;++i){
		for(j=0;j<trows;++j) {
			if (world[i*trows+j] == 1){
				caca_set_color(CACA_COLOR_LIGHTRED,CACA_COLOR_MAGENTA);
				caca_putchar(j,i,'@');
			}
			if (world[i*trows+j] == 0) {
				caca_set_color(CACA_COLOR_BLACK,CACA_COLOR_BLACK);
				caca_putchar(j,i,' ');
			}
		}
	}
	caca_refresh();
}

/********************************************************************/
void aff_finish() {
	caca_end();
}
#endif


/********************************************************************/
/**************************** affichage *****************************/
#ifdef USE_SDL
SDL_Surface *screen;

void aff_init(unsigned int * tlines, unsigned int * trows) {
	/*     unsigned int LINESSDL=1400; */
	/*     unsigned int ROWSSDL=1050; */
	unsigned int LINESSDL=640;
	unsigned int ROWSSDL=480;
	/*init SDL*/
	if (SDL_Init(SDL_INIT_VIDEO) != 0) {

		printf("Unable to initialize SDL: %s\n", SDL_GetError());
	}

	atexit(SDL_Quit);

	/*init surface*/

	/*     screen = SDL_SetVideoMode(LINESSDL, ROWSSDL, 8,  SDL_ANYFORMAT | SDL_FULLSCREEN); */
	screen = SDL_SetVideoMode(LINESSDL, ROWSSDL, 8,  SDL_ANYFORMAT);
	if (screen == NULL) {

		printf("Unable to set video mode: %s\n", SDL_GetError());
	}


	/* init taille du monde */
	*tlines = ROWSSDL; /* initialisé par ncurses */
	*trows  = LINESSDL;  /* initialisé par ncurses */
	/*     refresh(); */
}

/*http://www.libsdl.org/intro.fr/usingvideofr.html*/
void DrawPixel(SDL_Surface *screen, int x, int y,
		Uint8 R, Uint8 G,
		Uint8 B)
{
	Uint32 color = SDL_MapRGB(screen->format, R, G, B);
	switch (screen->format->BytesPerPixel) {
		case 1: {
				/*On gère le mode 8bpp */
				Uint8 *bufp;

				bufp = (Uint8 *)screen->pixels + y*screen->pitch + x;
				/*                     *bufp = (*bufp+(Uint8)color)/2; */
				*bufp = color;
			}
			break;

		case 2: {
				/* Certainement 15 ou 16 bpp */
				Uint16 *bufp;

				bufp = (Uint16 *)screen->pixels + y*screen->pitch/2 + x;
				/*                     *bufp = (*bufp+(Uint16)color)/2; */
				*bufp = color;
			}
			break;

		case 3: {
				/* 24 bpp lent et généralement pas utilisé */
				Uint8 *bufp;

				bufp = (Uint8 *)screen->pixels + y*screen->pitch + x * 3;
				if(SDL_BYTEORDER == SDL_LIL_ENDIAN) {

					bufp[0] = color;
					bufp[1] = color >> 8;
					bufp[2] = color >> 16;
				} else {

					bufp[2] = color;
					bufp[1] = color >> 8;
					bufp[0] = color >> 16;
				}
			}
			break;

		case 4: {
				/* Probablement 32 bpp alors */
				Uint32 *bufp;

				bufp = (Uint32 *)screen->pixels + y*screen->pitch/4 + x;
				*bufp = (*bufp/2+color);
				/*                     *bufp = color; */
			}
			break;
	}
}

/********************************************************************/
void aff_refresh(unsigned int * world, unsigned int tlines, unsigned int trows) {
	register unsigned int i,j;

	if ( SDL_MUSTLOCK(screen) ) {
		if ( SDL_LockSurface(screen) < 0 ) {
			return;
		}
	}
#pragma omp parallel for private(j) schedule(dynamic,1)
	for(i=0;i<tlines;++i){
		for(j=0;j<trows;++j) {
			if (world[i*trows+j] == 1){
				DrawPixel(screen, j,i, 255,0,0);
			}
			if (world[i*trows+j] == 0) {
				DrawPixel(screen, j,i, 0,0,0);
			}
		}
	}
	/*     SDL_UpdateRect(screen , 0 , 0 , 0 , 0 ); */
	if ( SDL_MUSTLOCK(screen) ) {

		SDL_UnlockSurface(screen);
	}
	SDL_Flip(screen);
	SDL_Event event;
	if (SDL_PollEvent(&event)&& (event.type==SDL_KEYDOWN)) finish(0);
}

/********************************************************************/
void aff_finish() {
	SDL_Quit();
}

#endif

