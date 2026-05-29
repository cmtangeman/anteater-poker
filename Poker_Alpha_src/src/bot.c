#include "bot.h"
#include "rules.h"

#include <stdio.h>

// Boiler plate function that evaluates the value of a hand and then 
// makes a move based on the value
// Function will be replaced with better logic that tracks other player hand values
ActionRequest botAction(Player *bot) {
    ActionRequest ar;

    // go all in for strongest hand value
    if(evaluateHand(bot->hand, HAND_SIZE) == 10) {
        // bot->currentBet = bot->chips;
        
        ar.action = ACTION_ALL_IN;

        bot->allIn = 1;
    }
    // if hand is strong, increase the bet amount
    if(evaluateHand(bot->hand, HAND_SIZE) > 8) {
        // bot->currentBet = .8 * bot->chips;

        ar.action = ACTION_RAISE;
    
    // if hand value is mediocre, continue with the bet amount  
    } else if(evaluateHand(bot->hand, HAND_SIZE) > 4) {
        ar.action = ACTION_CALL;

    // if hand value is poor, fold
    } else {
        bot->folded = 1;
        ar.action = ACTION_FOLD;
    }

    ar.amount = bot->currentBet;

    return ar;
} 


/*
// main for testing bot actions 
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
        10,
        0,
        0
    };

    ActionRequest ar = botAction(&bot);
    printf("%d %d\n", ar.action, ar.amount);
}
*/
