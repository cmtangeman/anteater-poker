#include "types.h"
#include "bot.h"
#include "rules.h"

#include <stdio.h>

// Boiler plate function that evaluates the value of a hand and then 
// makes a move based on the value
// Function will be replaced with better logic that tracks other player hand values
void botAction(Player *bot) {
    printf("%d\n", evaluateHand(bot->hand, HAND_SIZE));
    if(evaluateHand(bot->hand, HAND_SIZE) > 8) {
        printf("raise\n");
    } else if(evaluateHand(bot->hand, HAND_SIZE) > 4) {
        printf("call\n");
    } else {
        bot->folded = 1;
        printf("fold\n");
    }
} 

// main for testing bot actions 
/*
int main() {
    Player bot = {
        "Bot1",
        {
            {
                HEARTS,
                Anteater,
            },
            {
                HEARTS,
                ANT,
            }
        },
        STARTING_CHIPS,
        100,
        0,
        0
    };

    botAction(&bot);
}
    */