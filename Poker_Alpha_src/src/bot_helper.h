#ifndef BOT_HELPER_H
#define BOT_HELPER_H

#include "types.h"
#include "rules.h"

#include <stdlib.h>

typedef struct {
    float win_probability;
    float tie_probability;
    float loss_probability;
} EquityResult;

int card_eq(Card a, Card b);

Deck build_remaining_deck(Card *my_holes, int num_holes, Card *community, 
    int num_community);

void shuffle(Deck *d);

void complete_board(Card *community, int num_community, Deck *d, Card *out_board);

void deal_opponent(Deck *d, int deck_offset, Card *opp_holes);

HandRank evaluate_with_known(Card *hole_cards, Card *board, int board_size);

EquityResult monte_carlo(Card *my_holes, int num_holes,
    Card *community, int num_community,
    int num_opponents,
    int num_simulations);


#endif