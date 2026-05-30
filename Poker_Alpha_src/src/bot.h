#ifndef BOT_H
#define BOT_H

#include "types.h"
#include <stdlib.h>

#define MAX_HAND_RANGE 1770

ActionRequest botAction(Player *bot, GameState *gs); 

ActionRequest easyMode(Player *bot, Card *all_cards, int card_count);

ActionRequest medMode(Player *bot, Card *bot_cards, Card *comm_card, 
    int comm_card_count);

typedef struct {
    Card c1, c2;
    float weight;
} WeightedHand;

typedef struct {
    WeightedHand hands[MAX_HAND_RANGE];
    int count;
} Range;

#endif