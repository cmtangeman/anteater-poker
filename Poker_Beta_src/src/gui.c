#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <stdio.h>
#include "types.h"

#define FONT_PATH "/usr/share/fonts/google-droid/DroidSans-Bold.ttf"
#define WIN_W 1280
#define WIN_H 720

static const char *rank_str(Rank r) {
    static const char *names[] = {
        "ANT","2","3","4","5","6","7","8","9","10","J","Q","K","A","ANTE"
    };
    return names[r];
}

static const char *suit_str(Suit s) {
    static const char *names[] = {"H","D","C","S"};
    return names[s];
}

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

static void draw_card(SDL_Renderer *r, TTF_Font *f, int x, int y, Card c) {
    SDL_Rect card = { x, y, 70, 100 };
    SDL_SetRenderDrawColor(r, 250, 250, 250, 255);
    SDL_RenderFillRect(r, &card);
    SDL_SetRenderDrawColor(r, 0, 0, 0, 255);
    SDL_RenderDrawRect(r, &card);
    SDL_Color black = {0, 0, 0, 255};
    SDL_Color red   = {200, 30, 30, 255};
    SDL_Color col = (c.suit == HEARTS || c.suit == DIAMONDS) ? red : black;
    draw_text(r, f, rank_str(c.rank), x + 6, y + 4, col);
    draw_text(r, f, suit_str(c.suit), x + 6, y + 40, col);
}

static void draw_card_back(SDL_Renderer *r, int x, int y) {
    SDL_Rect card = { x, y, 70, 100 };
    SDL_SetRenderDrawColor(r, 80, 30, 30, 255);
    SDL_RenderFillRect(r, &card);
    SDL_SetRenderDrawColor(r, 200, 200, 200, 255);
    SDL_RenderDrawRect(r, &card);
}

static void draw_button(SDL_Renderer *r, TTF_Font *f, int x, int y,
                        const char *label) {
    SDL_Rect btn = { x, y, 140, 50 };
    SDL_SetRenderDrawColor(r, 60, 60, 70, 255);
    SDL_RenderFillRect(r, &btn);
    SDL_SetRenderDrawColor(r, 200, 200, 200, 255);
    SDL_RenderDrawRect(r, &btn);
    SDL_Color white = {255, 255, 255, 255};
    draw_text(r, f, label, x + 35, y + 15, white);
}

int main(void) {
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        fprintf(stderr, "SDL_Init failed: %s\n", SDL_GetError());
        return 1;
    }
    if (TTF_Init() != 0) {
        fprintf(stderr, "TTF_Init failed: %s\n", TTF_GetError());
        SDL_Quit();
        return 1;
    }

    SDL_Window *window = SDL_CreateWindow(
        "Anteater Poker - Client", SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED, WIN_W, WIN_H, SDL_WINDOW_SHOWN);
    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_SOFTWARE);

    TTF_Font *title_font = TTF_OpenFont(FONT_PATH, 48);
    TTF_Font *body_font  = TTF_OpenFont(FONT_PATH, 20);
    TTF_Font *card_font  = TTF_OpenFont(FONT_PATH, 18);
    if (!title_font || !body_font || !card_font) {
        fprintf(stderr, "TTF_OpenFont failed: %s\n", TTF_GetError());
        return 1;
    }

    Card hole_cards[2] = {{HEARTS, ACE}, {SPADES, KING}};
    Card community[5]  = {{CLUBS, TEN}, {HEARTS, JACK}, {DIAMONDS, QUEEN},
                          {SPADES, NINE}, {HEARTS, SEVEN}};

    int running = 1;
    while (running) {
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) running = 0;
            if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_ESCAPE) running = 0;
        }

        SDL_SetRenderDrawColor(renderer, 14, 92, 46, 255);
        SDL_RenderClear(renderer);

        SDL_Color white = {255, 255, 255, 255};
        SDL_Color gold  = {230, 200, 60, 255};

        draw_text(renderer, title_font, "ANTEATER POKER", 430, 20, gold);
        draw_text(renderer, body_font,  "Status: Connecting to server...", 30, 90, white);
        draw_text(renderer, body_font,  "Pot: $0", WIN_W - 200, 90, white);

        for (int i = 0; i < 5; i++)
            draw_card(renderer, card_font, 350 + i*100, 280, community[i]);

        draw_card_back(renderer, 100, 180);
        draw_card_back(renderer, 180, 180);
        draw_text(renderer, body_font, "Bot 1: $20", 100, 290, white);

        draw_card_back(renderer, WIN_W - 250, 180);
        draw_card_back(renderer, WIN_W - 170, 180);
        draw_text(renderer, body_font, "Bot 2: $20", WIN_W - 250, 290, white);

        draw_card(renderer, card_font, 540, 500, hole_cards[0]);
        draw_card(renderer, card_font, 620, 500, hole_cards[1]);
        draw_text(renderer, body_font, "You: $20", 540, 610, white);

        draw_button(renderer, body_font, 850, 530, "FOLD");
        draw_button(renderer, body_font, 1000, 530, "CALL");
        draw_button(renderer, body_font, 1150, 530, "RAISE");

        SDL_RenderPresent(renderer);
    }

    TTF_CloseFont(title_font);
    TTF_CloseFont(body_font);
    TTF_CloseFont(card_font);
    TTF_Quit();
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}

