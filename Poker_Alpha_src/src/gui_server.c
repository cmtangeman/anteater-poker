#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <stdio.h>

#define FONT_PATH "/usr/share/fonts/google-droid/DroidSans-Bold.ttf"
#define WIN_W 1024
#define WIN_H 600

static void draw_text(SDL_Renderer *r, TTF_Font *f, const char *text,
                        int x, int y, SDL_Color color) {
      SDL_Surface *surf = TTF_RenderText_Blended(f, text, color);
      if (!surf) return;
      SDL_Texture *tex = SDL_CreateTextureFromSurface(r, surf);
      SDL_Rect dst = { x, y, surf->w, surf->h };
      SDL_RenderCopy(r, tex, NULL, &dst);
      SDL_FreeSurface(surf);
      SDL_DestroyTexture(tex);
}

int main(void) {
      if (SDL_Init(SDL_INIT_VIDEO) != 0) return 1;
      if (TTF_Init() != 0) return 1;

      SDL_Window *win = SDL_CreateWindow("Anteater Poker - Server Control",
          SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
          WIN_W, WIN_H, SDL_WINDOW_SHOWN);
      SDL_Renderer *ren = SDL_CreateRenderer(win, -1, SDL_RENDERER_SOFTWARE);

      TTF_Font *title = TTF_OpenFont(FONT_PATH, 36);
      TTF_Font *body  = TTF_OpenFont(FONT_PATH, 20);

      int running = 1;
      while (running) {
          SDL_Event e;
          while (SDL_PollEvent(&e)) {
              if (e.type == SDL_QUIT) running = 0;
              if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_ESCAPE) running = 0;
          }

          SDL_SetRenderDrawColor(ren, 25, 25, 40, 255);
          SDL_RenderClear(ren);

          SDL_Color white = {230, 230, 230, 255};
          SDL_Color green = {100, 220, 130, 255};

          draw_text(ren, title, "ANTEATER POKER - SERVER", 220, 30, green);
          draw_text(ren, body, "Status: Listening on port 8080", 60, 130, white);
          draw_text(ren, body, "Connected Players: 0 / 6",      60, 170, white);
          draw_text(ren, body, "Current Phase: WAITING",        60, 210, white);
          draw_text(ren, body, "Pot: $0",                       60, 250, white);
          draw_text(ren, body, "Hand #: 0",                     60, 290, white);

          draw_text(ren, body, "--- Player List ---",           60, 360, white);
          draw_text(ren, body, "(no players yet)",              60, 400, white);

          draw_text(ren, body, "Press ESC to shut down server.",60, 540, white);

          SDL_RenderPresent(ren);
      }

      TTF_CloseFont(title);
      TTF_CloseFont(body);
      TTF_Quit();
      SDL_DestroyRenderer(ren);
      SDL_DestroyWindow(win);
      SDL_Quit();
      return 0;
}
