#ifndef BOT_H
#define BOT_H

#include "types.h"
#include <stdlib.h>

#define MAX_HAND_RANGE 1770

ActionRequest botAction(Player *bot); 

typedef struct {
    Card c1, c2;
    float weight;
} WeightedHand;

typedef struct {
    WeightedHand hands[MAX_HAND_RANGE];
    int count;
} Range;

#endif