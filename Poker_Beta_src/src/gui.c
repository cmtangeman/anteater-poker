#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <errno.h>
#include "types.h"

#define FONT_PATH "/usr/share/fonts/google-droid/DroidSans-Bold.ttf"
#define WIN_W 1280
#define WIN_H 720

typedef enum {
    CONV_WAIT_NAME_PROMPT,
    CONV_WAIT_WELCOME,
    CONV_PLAYING
} ConvState;

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

static int connect_to_server(const char *hostname, int port) {
    struct hostent *server = gethostbyname(hostname);
    if (!server) {
        fprintf(stderr, "gethostbyname failed for '%s'\n", hostname);
        return -1;
    }

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr = *(struct in_addr*)server->h_addr_list[0];

    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("socket");
        return -1;
    }

    if (connect(sock, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("connect");
        close(sock);
        return -1;
    }

    fcntl(sock, F_SETFL, O_NONBLOCK);
    return sock;
}

static void send_line(int sock, const char *msg) {
    write(sock, msg, strlen(msg));
    printf("[gui->server] %s\n", msg);
    fflush(stdout);
}

int main(int argc, char *argv[]) {
    if (argc < 3) {
        fprintf(stderr, "Usage: %s hostname port\n", argv[0]);
        return 1;
    }
    int port = atoi(argv[2]);
    if (port <= 2000) {
        fprintf(stderr, "Invalid port %d (must be > 2000)\n", port);
        return 1;
    }

    int sock = connect_to_server(argv[1], port);
    if (sock < 0) {
        fprintf(stderr, "Could not connect to %s:%d\n", argv[1], port);
        return 1;
    }
    printf("[gui] Connected to %s:%d\n", argv[1], port);
    fflush(stdout);

    send_line(sock, "START");

    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        fprintf(stderr, "SDL_Init failed: %s\n", SDL_GetError());
        close(sock);
        return 1;
    }
    if (TTF_Init() != 0) {
        fprintf(stderr, "TTF_Init failed: %s\n", TTF_GetError());
        SDL_Quit();
        close(sock);
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
        close(sock);
        return 1;
    }

    Card hole_cards[2] = {{HEARTS, ACE}, {SPADES, KING}};
    Card community[5]  = {{CLUBS, TEN}, {HEARTS, JACK}, {DIAMONDS, QUEEN},
                          {SPADES, NINE}, {HEARTS, SEVEN}};

    char status_line[256] = "Sent START, waiting for name prompt...";
    ConvState conv = CONV_WAIT_NAME_PROMPT;

    int running = 1;
    while (running) {
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) running = 0;
            if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_ESCAPE) running = 0;
        }

        char netbuf[2048];
        int n = read(sock, netbuf, sizeof(netbuf)-1);
        if (n > 0) {
            netbuf[n] = 0;
            printf("[server] %s\n", netbuf);
            fflush(stdout);

            if (conv == CONV_WAIT_NAME_PROMPT && strstr(netbuf, "NAME")) {
                send_line(sock, "GuiTest");
                conv = CONV_WAIT_WELCOME;
                snprintf(status_line, sizeof(status_line),
                         "Sent name GuiTest - waiting for welcome");
            } else if (conv == CONV_WAIT_WELCOME && strstr(netbuf, "Welcome")) {
                send_line(sock, "READY");
                conv = CONV_PLAYING;
                snprintf(status_line, sizeof(status_line), "Ready! In game.");
            } else {
                char copy[220];
                size_t len = strlen(netbuf);
                if (len > 200) len = 200;
                memcpy(copy, netbuf, len);
                copy[len] = 0;
                for (char *p = copy; *p; p++) if (*p == '\n') *p = ' ';
                snprintf(status_line, sizeof(status_line), "Last: %.200s", copy);
            }
        } else if (n == 0) {
            printf("[gui] Server closed the connection.\n");
            fflush(stdout);
            snprintf(status_line, sizeof(status_line), "Server disconnected.");
            running = 0;
        } else if (errno != EAGAIN && errno != EWOULDBLOCK) {
            perror("read");
            snprintf(status_line, sizeof(status_line), "Socket error.");
            running = 0;
        }

        SDL_SetRenderDrawColor(renderer, 14, 92, 46, 255);
        SDL_RenderClear(renderer);

        SDL_Color white = {255, 255, 255, 255};
        SDL_Color gold  = {230, 200, 60, 255};

        draw_text(renderer, title_font, "ANTEATER POKER", 430, 20, gold);
        draw_text(renderer, body_font,  status_line, 30, 90, white);
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
        SDL_Delay(16);
    }

    close(sock);
    TTF_CloseFont(title_font);
    TTF_CloseFont(body_font);
    TTF_CloseFont(card_font);
    TTF_Quit();
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}

