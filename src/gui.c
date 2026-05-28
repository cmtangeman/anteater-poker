#include <SDL2/SDL.h>
#include <stdio.h>

int main(void) {
	if (SDL_Init(SDL_INIT_VIDEO) != 0) {
		fprintf(stderr, "SDL2 load failed: %s\n", SDL_GetError());
		return 1;
	}

	SDL_Window *window = SDL_CreateWindow(
			"Anteater Poker",
			SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
			1080, 608,
			SDL_WINDOW_SHOWN);
	
	SDL_Renderer *renderer = SDL_CreateRenderer(
			window, -1, SDL_RENDERER_SOFTWARE);

	int running = 1;
	while (running) {
		SDL_Event e;
		while (SDL_PollEvent(&e)) {
			if (e.type == SDL_QUIT) running = 0;
              		if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_ESCAPE) running = 0;
		}

		SDL_SetRenderDrawColor(renderer, 53, 101, 77, 255);
		SDL_RenderClear(renderer);
		SDL_RenderPresent(renderer);
	}

	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
	return 0;
}
