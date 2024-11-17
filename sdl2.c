#include <sys/time.h>

#include <SDL2/SDL.h>

#include <pthread.h>
#include <stdlib.h>
#include <errno.h>
#include <err.h>

#include "engine.h"
#include "engine-ocl.h"

static bool running = true;
static SDL_Window *win;
static SDL_Renderer *ren;

static pthread_t input_thread;

void *sdl_loop(void *np) {
	SDL_Event event;

	while (running) {
		SDL_PollEvent(&event);
		if (event.type == SDL_QUIT)
			running = false;
		if (event.type == SDL_KEYDOWN) {
			switch (event.key.keysym.sym) {
			case SDLK_r:
				randomize_space();
				break;
			default:
				break;
			}
		}
	}

	SDL_DestroyRenderer(ren);
	SDL_Quit();
	return NULL;
}

void update() {
	update_space();
}

void draw() {
	if (win == NULL) {
	}

	int n, col;
	int r, g, b;
	SDL_Rect rect = {0, 0, PIXEL_SIZE, PIXEL_SIZE};
	for (int i = 0; i < X; i++)
		for (int j = 0; j < Y; j++) {
			n = 0;
			for (int k = 0; k < Z; k++)
				if (space[TABLE_INDEX(i, j, k)])
					n++;

			col = 0xffffff*n/Z;
			r = (col>>16)&0xff;
			g = (col>>8)&0xff;
			b = col&0xff;

			rect.x = i*PIXEL_SIZE;
			rect.y = j*PIXEL_SIZE;
			SDL_SetRenderDrawColor(ren, r, g, b, 0xff);
			SDL_RenderFillRect(ren, &rect);
		}
	
	SDL_RenderPresent(ren);
}

int main() {
	init_opencl();
	puts("Init SDL");
	if (SDL_Init(SDL_INIT_EVERYTHING) != 0)
		err(errno, SDL_GetError());

	puts("randomize space");
	randomize_space();

	puts("Creating window");
	win = SDL_CreateWindow("", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, X*PIXEL_SIZE, Y*PIXEL_SIZE, SDL_WINDOW_SHOWN);
	ren = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED);

	puts("Creating input thread");
	pthread_create(&input_thread, NULL, sdl_loop, NULL);

	puts("Entering main loop");
	struct timeval tv_current, tv_last_draw;
	gettimeofday(&tv_last_draw, NULL);
	draw();
	while (running) {
		update();

		// Draw at approximately 100Hz
		gettimeofday(&tv_current, NULL);
		if (tv_current.tv_usec - tv_last_draw.tv_usec > 10000
		    || tv_current.tv_sec != tv_last_draw.tv_sec) {
			write_space();
			draw();
			tv_current = tv_last_draw;
		}
	}

	free_opencl();
}
