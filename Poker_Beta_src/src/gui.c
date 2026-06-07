/* EECS 22L Project 2 - Anteater Poker
 * GUI client (Team 18)
 * Connects to the team's text protocol, parses sendPlayerStatus blocks,
 * renders the table in SDL2, and sends 1-5 action codes back on click.
 *
 * Protocol expected from the server (see poker_server.c sendPlayerStatus):
 *   --- <PhaseName> ---
 *   [Community: <R> of <S>  <R> of <S>  ...]
 *   <name>'s STATUS:
 *   Hand: <R> of <S>, <R> of <S>
 *   Chips: $<int>
 *   Pot: $<int> | Current Bet: $<int>
 *   1)Check 2)Call 3)Bet 4)Fold 5)AllIn
 *
 * Action wire format:
 *   Check=1, Call=2, Bet=3, Fold=4, AllIn=5  (no trailing newline)
 *   Bet is two messages: send "3", then send the amount as a separate write.
 */

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <errno.h>
#include "types.h"

#define FONT_PATH "/usr/share/fonts/google-droid/DroidSans-Bold.ttf"
#define WIN_W 1280
#define WIN_H 720
#define MY_NAME "Greg"

typedef enum {
    HS_WAIT_NAME_PROMPT,
    HS_WAIT_WELCOME,
    HS_PLAYING
} HandshakeState;

typedef struct {
    int   phase;
    char  phase_name[32];
    Card  hole[2];
    int   hole_known;
    Card  community[5];
    int   community_count;
    int   my_chips;
    int   pot;
    int   current_bet;
    int   my_turn;
    int   in_game;
    int   round_over;
    int   bet_amount;
    char  status_msg[256];
} GuiState;

typedef struct {
    int x, y, w, h;
    const char *label;
    int code;     /* 1-5 = server action codes; -1 = bet-, -2 = bet+ */
} Button;

static int g_sock = -1;

/* Must match poker_server.c rankNames/suitNames exactly */
static const char *rank_names_full[] = {
    "Ant","2","3","4","5","6","7","8","9","10","J","Q","K","A","Anteater"
};
static const char *suit_names_full[] = {"Hearts","Diamonds","Clubs","Spades"};

/* Short labels for cards drawn on screen */
static const char *rank_short(Rank r) {
    static const char *n[] = {
        "Ant","2","3","4","5","6","7","8","9","10","J","Q","K","A","ANT!"
    };
    return n[r];
}
static const char *suit_short(Suit s) {
    static const char *n[] = {"H","D","C","S"};
    return n[s];
}

static int rank_from_name(const char *s) {
    for (int i = 0; i < 15; i++)
        if (strcmp(s, rank_names_full[i]) == 0) return i;
    return -1;
}
static int suit_from_name(const char *s) {
    for (int i = 0; i < 4; i++)
        if (strcmp(s, suit_names_full[i]) == 0) return i;
    return -1;
}

static int parse_card(const char *s, Card *out) {
    char r[16] = {0}, u[16] = {0};
    if (sscanf(s, " %15s of %15s", r, u) != 2) return 0;
    int ri = rank_from_name(r);
    int ui = suit_from_name(u);
    if (ri < 0 || ui < 0) return 0;
    out->rank = (Rank)ri;
    out->suit = (Suit)ui;
    return 1;
}

static void parse_server_text(GuiState *g, const char *buf) {
    const char *p = buf;
    while (*p) {
        const char *eol = strchr(p, '\n');
        size_t len = eol ? (size_t)(eol - p) : strlen(p);
        char line[512];
        if (len >= sizeof(line)) len = sizeof(line) - 1;
        memcpy(line, p, len);
        line[len] = 0;
        if (len > 0 && line[len-1] == '\r') line[len-1] = 0;

        if (strncmp(line, "--- ", 4) == 0) {
            char phase[32] = {0};
            sscanf(line + 4, "%31s", phase);
            strncpy(g->phase_name, phase, sizeof(g->phase_name) - 1);
            const char *ph[] = {
                "Waiting","Preflop","Flop","Turn","River","Showdown","Over"
            };
            for (int i = 0; i < 7; i++)
                if (strcmp(phase, ph[i]) == 0) { g->phase = i; break; }
            g->in_game = 1;
        }
        else if (strncmp(line, "Community: ", 11) == 0) {
            g->community_count = 0;
            const char *q = line + 11;
            while (*q && g->community_count < 5) {
                while (*q == ' ') q++;
                if (!*q) break;
                const char *end = strstr(q, "  ");
                if (!end) end = q + strlen(q);
                char cb[32] = {0};
                size_t l = (size_t)(end - q);
                if (l >= sizeof(cb)) l = sizeof(cb) - 1;
                memcpy(cb, q, l);
                cb[l] = 0;
                Card c;
                if (parse_card(cb, &c)) g->community[g->community_count++] = c;
                if (*end == 0) break;
                q = end + 2;
            }
        }
        else if (strncmp(line, "Hand: ", 6) == 0) {
            const char *body = line + 6;
            const char *comma = strchr(body, ',');
            if (comma) {
                char a[32] = {0};
                size_t l = (size_t)(comma - body);
                if (l >= sizeof(a)) l = sizeof(a) - 1;
                memcpy(a, body, l);
                a[l] = 0;
                const char *b = comma + 1;
                Card c1, c2;
                if (parse_card(a, &c1) && parse_card(b, &c2)) {
                    g->hole[0] = c1;
                    g->hole[1] = c2;
                    g->hole_known = 1;
                }
            }
        }
        else if (strncmp(line, "Chips: $", 8) == 0) {
            g->my_chips = atoi(line + 8);
        }
        else if (strncmp(line, "Pot: $", 6) == 0) {
            int pot = 0, cur = 0;
            sscanf(line, "Pot: $%d | Current Bet: $%d", &pot, &cur);
            g->pot = pot;
            g->current_bet = cur;
        }
        else if (strstr(line, "1)Check")) {
            g->my_turn = 1;
            g->in_game = 1;
            g->round_over = 0;
            if (g->bet_amount < g->current_bet + 1)
                g->bet_amount = g->current_bet + 1;
            if (g->bet_amount > g->my_chips) g->bet_amount = g->my_chips;
            if (g->bet_amount < 1) g->bet_amount = 1;
            snprintf(g->status_msg, sizeof(g->status_msg), "Your turn.");
        }
        else if (strstr(line, "is the winner") || strstr(line, "Winner:")) {
            g->my_turn = 0;
            g->round_over = 1;
            snprintf(g->status_msg, sizeof(g->status_msg), "%.250s", line);
        }
        else if (strstr(line, "Not your turn")) {
            g->my_turn = 0;
            snprintf(g->status_msg, sizeof(g->status_msg), "Not your turn.");
        }

        if (!eol) break;
        p = eol + 1;
    }
}

static int connect_to_server(const char *hostname, int port) {
    struct hostent *srv = gethostbyname(hostname);
    if (!srv) {
        fprintf(stderr, "gethostbyname failed for '%s'\n", hostname);
        return -1;
    }
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr = *(struct in_addr *)srv->h_addr_list[0];
    int s = socket(AF_INET, SOCK_STREAM, 0);
    if (s < 0) { perror("socket"); return -1; }
    if (connect(s, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("connect");
        close(s);
        return -1;
    }
    /* Disable Nagle so the two-write BET sequence doesn't coalesce */
    int one = 1;
    setsockopt(s, IPPROTO_TCP, TCP_NODELAY, &one, sizeof(one));
    fcntl(s, F_SETFL, O_NONBLOCK);
    return s;
}

static void send_msg(const char *s) {
    write(g_sock, s, strlen(s));
    printf("[gui->server] %s\n", s);
    fflush(stdout);
}

static void send_action(int code, int amount) {
    char buf[16];
    snprintf(buf, sizeof(buf), "%d", code);
    send_msg(buf);
    if (code == 3) {
        /* Server does a 2nd blocking recvMsg() for the amount */
        SDL_Delay(80);
        snprintf(buf, sizeof(buf), "%d", amount);
        send_msg(buf);
    }
}

static void draw_text(SDL_Renderer *r, TTF_Font *f, const char *t,
                      int x, int y, SDL_Color c) {
    if (!t || !*t) return;
    SDL_Surface *s = TTF_RenderText_Blended(f, t, c);
    if (!s) return;
    SDL_Texture *tex = SDL_CreateTextureFromSurface(r, s);
    SDL_Rect dst = { x, y, s->w, s->h };
    SDL_RenderCopy(r, tex, NULL, &dst);
    SDL_FreeSurface(s);
    SDL_DestroyTexture(tex);
}

static void draw_card(SDL_Renderer *r, TTF_Font *f, int x, int y, Card c) {
    SDL_Rect rc = { x, y, 70, 100 };
    SDL_SetRenderDrawColor(r, 250, 250, 250, 255);
    SDL_RenderFillRect(r, &rc);
    SDL_SetRenderDrawColor(r, 0, 0, 0, 255);
    SDL_RenderDrawRect(r, &rc);
    SDL_Color black = {0, 0, 0, 255};
    SDL_Color red   = {200, 30, 30, 255};
    SDL_Color col = (c.suit == HEARTS || c.suit == DIAMONDS) ? red : black;
    draw_text(r, f, rank_short(c.rank), x + 6, y + 4, col);
    draw_text(r, f, suit_short(c.suit), x + 6, y + 40, col);
}

static void draw_card_back(SDL_Renderer *r, int x, int y) {
    SDL_Rect rc = { x, y, 70, 100 };
    SDL_SetRenderDrawColor(r, 80, 30, 30, 255);
    SDL_RenderFillRect(r, &rc);
    SDL_SetRenderDrawColor(r, 200, 200, 200, 255);
    SDL_RenderDrawRect(r, &rc);
}

static void draw_card_slot(SDL_Renderer *r, int x, int y) {
    SDL_Rect rc = { x, y, 70, 100 };
    SDL_SetRenderDrawColor(r, 10, 60, 30, 255);
    SDL_RenderFillRect(r, &rc);
    SDL_SetRenderDrawColor(r, 50, 110, 70, 255);
    SDL_RenderDrawRect(r, &rc);
}

static void draw_button(SDL_Renderer *r, TTF_Font *f, const Button *b,
                        int enabled, int hover) {
    SDL_Rect rc = { b->x, b->y, b->w, b->h };
    if (!enabled)   SDL_SetRenderDrawColor(r, 40, 40, 45, 255);
    else if (hover) SDL_SetRenderDrawColor(r, 90, 90, 110, 255);
    else            SDL_SetRenderDrawColor(r, 60, 60, 75, 255);
    SDL_RenderFillRect(r, &rc);
    int edge = enabled ? 220 : 100;
    SDL_SetRenderDrawColor(r, edge, edge, edge, 255);
    SDL_RenderDrawRect(r, &rc);
    int tc = enabled ? 255 : 140;
    SDL_Color tx = { tc, tc, tc, 255 };
    int tw = 0, th = 0;
    TTF_SizeText(f, b->label, &tw, &th);
    draw_text(r, f, b->label, b->x + (b->w - tw)/2, b->y + (b->h - th)/2, tx);
}

static int hit(const Button *b, int mx, int my) {
    return mx >= b->x && mx < b->x + b->w
        && my >= b->y && my < b->y + b->h;
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

    g_sock = connect_to_server(argv[1], port);
    if (g_sock < 0) {
        fprintf(stderr, "Could not connect to %s:%d\n", argv[1], port);
        return 1;
    }
    printf("[gui] Connected to %s:%d\n", argv[1], port);
    fflush(stdout);

    send_msg("START");

    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        fprintf(stderr, "SDL_Init: %s\n", SDL_GetError());
        close(g_sock);
        return 1;
    }
    if (TTF_Init() != 0) {
        fprintf(stderr, "TTF_Init: %s\n", TTF_GetError());
        SDL_Quit();
        close(g_sock);
        return 1;
    }

    SDL_Window *win = SDL_CreateWindow(
        "Anteater Poker - Client",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        WIN_W, WIN_H, SDL_WINDOW_SHOWN);
    SDL_Renderer *ren = SDL_CreateRenderer(win, -1, SDL_RENDERER_SOFTWARE);

    TTF_Font *title_font = TTF_OpenFont(FONT_PATH, 48);
    TTF_Font *body_font  = TTF_OpenFont(FONT_PATH, 20);
    TTF_Font *card_font  = TTF_OpenFont(FONT_PATH, 18);
    TTF_Font *btn_font   = TTF_OpenFont(FONT_PATH, 18);
    if (!title_font || !body_font || !card_font || !btn_font) {
        fprintf(stderr, "TTF_OpenFont: %s\n", TTF_GetError());
        close(g_sock);
        return 1;
    }

    GuiState gs;
    memset(&gs, 0, sizeof(gs));
    gs.bet_amount = 1;
    snprintf(gs.status_msg, sizeof(gs.status_msg),
             "Sent START - waiting for name prompt...");

    HandshakeState hs = HS_WAIT_NAME_PROMPT;

    /* Action button row at y=640. Codes 1-5 = server actions, -1/-2 = bet-/+ */
    Button buttons[7] = {
        { 280, 640, 100, 50, "CHECK",  1 },
        { 390, 640, 100, 50, "CALL",   2 },
        { 500, 640, 100, 50, "BET",    3 },
        { 610, 640, 100, 50, "FOLD",   4 },
        { 720, 640, 100, 50, "ALL-IN", 5 },
        { 850, 640,  40, 50, "-",     -1 },
        { 970, 640,  40, 50, "+",     -2 }
    };

    int running = 1;
    while (running) {
        int mx = 0, my = 0;
        SDL_GetMouseState(&mx, &my);

        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) running = 0;
            if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_ESCAPE)
                running = 0;
            if (e.type == SDL_MOUSEBUTTONDOWN &&
                e.button.button == SDL_BUTTON_LEFT) {
                for (int i = 0; i < 7; i++) {
                    if (!hit(&buttons[i], e.button.x, e.button.y)) continue;
                    int code = buttons[i].code;
                    if (code == -1) {
                        if (gs.bet_amount > 1) gs.bet_amount--;
                    } else if (code == -2) {
                        if (gs.bet_amount < gs.my_chips) gs.bet_amount++;
                    } else {
                        if (!gs.my_turn) {
                            snprintf(gs.status_msg, sizeof(gs.status_msg),
                                     "Wait for your turn...");
                            break;
                        }
                        send_action(code, gs.bet_amount);
                        gs.my_turn = 0;
                        snprintf(gs.status_msg, sizeof(gs.status_msg),
                                 "Sent %s. Waiting...", buttons[i].label);
                    }
                    break;
                }
            }
        }

        char netbuf[4096];
        int n = read(g_sock, netbuf, sizeof(netbuf) - 1);
        if (n > 0) {
            netbuf[n] = 0;
            printf("[server] %s\n", netbuf);
            fflush(stdout);

            if (hs == HS_WAIT_NAME_PROMPT && strstr(netbuf, "NAME")) {
                send_msg(MY_NAME);
                hs = HS_WAIT_WELCOME;
                snprintf(gs.status_msg, sizeof(gs.status_msg),
                         "Sent name %s - waiting for welcome", MY_NAME);
            } else if (hs == HS_WAIT_WELCOME && strstr(netbuf, "Welcome")) {
                send_msg("READY");
                hs = HS_PLAYING;
                snprintf(gs.status_msg, sizeof(gs.status_msg),
                         "Ready! Waiting for game to start...");
            } else if (hs == HS_PLAYING) {
                parse_server_text(&gs, netbuf);
            }
        } else if (n == 0) {
            printf("[gui] Server closed the connection.\n");
            fflush(stdout);
            snprintf(gs.status_msg, sizeof(gs.status_msg),
                     "Server disconnected.");
            running = 0;
        } else if (errno != EAGAIN && errno != EWOULDBLOCK) {
            perror("read");
            snprintf(gs.status_msg, sizeof(gs.status_msg), "Socket error.");
            running = 0;
        }

        /* ---------- render ---------- */
        SDL_SetRenderDrawColor(ren, 14, 92, 46, 255);
        SDL_RenderClear(ren);

        SDL_Color white = {255, 255, 255, 255};
        SDL_Color gold  = {230, 200, 60, 255};
        SDL_Color dim   = {180, 180, 180, 255};
        SDL_Color yel   = {255, 235, 100, 255};

        draw_text(ren, title_font, "ANTEATER POKER", 410, 5, gold);

        char phaseline[64];
        snprintf(phaseline, sizeof(phaseline), "Phase: %s",
                 gs.phase_name[0] ? gs.phase_name : "Waiting");
        draw_text(ren, body_font, phaseline, 30, 25, dim);

        char potline[64];
        snprintf(potline, sizeof(potline),
                 "Pot: $%d   Current Bet: $%d", gs.pot, gs.current_bet);
        draw_text(ren, body_font, potline, WIN_W - 320, 25, white);

        /* Bots in a semicircle across the top */
        int bot_xs[5] = {  60, 290, 570, 850, 1080 };
        int bot_ys[5] = { 130, 100,  90, 100,  130 };
        for (int i = 0; i < 5; i++) {
            draw_card_back(ren, bot_xs[i], bot_ys[i]);
            draw_card_back(ren, bot_xs[i] + 25, bot_ys[i] + 10);
            char lbl[16];
            snprintf(lbl, sizeof(lbl), "Bot %d", i + 1);
            draw_text(ren, body_font, lbl, bot_xs[i] + 15, bot_ys[i] + 115,
                      white);
        }

        /* Community cards centered */
        int cc_total_w = 5 * 80 - 10;
        int cc_start_x = (WIN_W - cc_total_w) / 2;
        for (int i = 0; i < 5; i++) {
            int x = cc_start_x + i * 80;
            if (i < gs.community_count)
                draw_card(ren, card_font, x, 290, gs.community[i]);
            else
                draw_card_slot(ren, x, 290);
        }

        /* Player hole cards bottom-center */
        if (gs.hole_known) {
            draw_card(ren, card_font, WIN_W/2 - 80, 460, gs.hole[0]);
            draw_card(ren, card_font, WIN_W/2 + 10, 460, gs.hole[1]);
        } else {
            draw_card_back(ren, WIN_W/2 - 80, 460);
            draw_card_back(ren, WIN_W/2 + 10, 460);
        }
        char chipline[64];
        snprintf(chipline, sizeof(chipline), "You (%s): $%d", MY_NAME,
                 gs.my_chips);
        draw_text(ren, body_font, chipline, WIN_W/2 - 80, 570, white);

        /* Turn indicator */
        if (gs.my_turn) {
            int tw = 0, th = 0;
            TTF_SizeText(body_font, ">>> YOUR TURN <<<", &tw, &th);
            draw_text(ren, body_font, ">>> YOUR TURN <<<",
                      (WIN_W - tw)/2, 600, yel);
        } else if (gs.in_game && !gs.round_over) {
            int tw = 0, th = 0;
            TTF_SizeText(body_font, "Waiting for other players...", &tw, &th);
            draw_text(ren, body_font, "Waiting for other players...",
                      (WIN_W - tw)/2, 600, dim);
        }

        /* Action buttons + bet amount widget */
        for (int i = 0; i < 7; i++) {
            int enabled = gs.my_turn;
            int hover = hit(&buttons[i], mx, my);
            draw_button(ren, btn_font, &buttons[i], enabled, hover);
        }
        /* Bet amount value sits between the "-" and "+" buttons */
        char betline[24];
        snprintf(betline, sizeof(betline), "$%d", gs.bet_amount);
        int btw = 0, bth = 0;
        TTF_SizeText(body_font, betline, &btw, &bth);
        draw_text(ren, body_font, betline,
                  930 - btw/2, 640 + (50 - bth)/2,
                  gs.my_turn ? white : dim);

        /* Status line at the bottom */
        draw_text(ren, body_font, gs.status_msg, 20, WIN_H - 28, white);

        SDL_RenderPresent(ren);
        SDL_Delay(16);
    }

    close(g_sock);
    TTF_CloseFont(title_font);
    TTF_CloseFont(body_font);
    TTF_CloseFont(card_font);
    TTF_CloseFont(btn_font);
    TTF_Quit();
    SDL_DestroyRenderer(ren);
    SDL_DestroyWindow(win);
    SDL_Quit();
    return 0;
}
