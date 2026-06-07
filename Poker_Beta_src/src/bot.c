#include "bot.h"
#include "rules.h"
#include "bot_helper.h"

#include <stdio.h>

// Function that determines the actions of the bot in the poker game 
ActionRequest botAction(Player *bot, GameState *gs) {
    int i, card_count;

    // set total no of cards to sum of hand size and community card count
    card_count = HAND_SIZE + gs->communityCardCount;

    // init the all_cards array to store all curret cards so that hand can be 
    // evaluated 
    Card all_cards[HAND_SIZE + COMMUNITY_CARDS] = {bot->hand[0], bot->hand[1]};
    for(i=HAND_SIZE;i<card_count;i++) {
        all_cards[i] = gs->communityCards[i-HAND_SIZE];
    }

    // if no bet has been made yet, make a default bet
    if(gs->currentBet == 0) {
        ActionRequest ar = {ACTION_BET, BOT_BET_AMT};
        return ar;
    }

    // return easyMode(bot, all_cards, card_count);
    return medMode(bot, bot->hand, gs->communityCards, gs->communityCardCount);
}


// Function that performs actions corresponding to the easy mode for the bot
ActionRequest easyMode(Player *bot, Card *all_cards, int card_count) {
    ActionRequest ar;

    // go all in for strongest hand value
    if(evaluateHand(all_cards, card_count) == 9) {
        ar.amount = bot->currentBet;    
        ar.action = ACTION_ALL_IN;
        bot->allIn = 1;
    }
    // if hand is strong, increase the bet amount
    else if(evaluateHand(all_cards, card_count) > 7) {
        ar.action = ACTION_RAISE;
    
    // if hand value is mediocre, continue with the bet amount  
    } else if(evaluateHand(all_cards, card_count) > 4) {
        ar.action = ACTION_CALL;

    // if hand value is poor, fold
    } else {
        bot->folded = 1;
        ar.action = ACTION_FOLD;
    }

    ar = botAmtforAction(ar.action, bot->chips, bot->currentBet);

    return ar;
}


// Function that performs actions corresponding to the medium mode for the bot
ActionRequest medMode(Player *bot, Card *bot_cards, Card *comm_card, 
    int comm_card_count) {

    ActionRequest ar;
    int opps_no;

    // TEMP HARD-CODED OPP VALUE
    // REPLACE WITH PLAYER COUNT
    opps_no = 1;

    // call the monte carlo function
    EquityResult eq = monte_carlo(bot_cards, HAND_SIZE, comm_card, comm_card_count,
        opps_no, 10000);
    
    // TEMP PRINT OF PROBS
    // printf("win: %f; tie: %f; loss:%f\n", eq.win_probability, eq.tie_probability,
    // eq.loss_probability);

    // if win rate above 90%, go all in
    if(eq.win_probability > 0.9) {
        ar.action = ACTION_ALL_IN;

        bot->allIn = 1;
    } else if(eq.win_probability > 0.7) {
        ar.action = ACTION_RAISE;
    } else if(eq.win_probability > 0.5) {
        ar.action = ACTION_CALL;
    } else {
        ar.action = ACTION_FOLD;

        bot->folded = 1;
    }

    ar = botAmtforAction(ar.action, bot->chips, bot->currentBet);

    return ar;
}

// Function that returns the amt that is to to bet for the current action of the
// bot
ActionRequest botAmtforAction(PlayerAction action, int chips, int currentBet) {
    ActionRequest ar;

    switch(action) {
        case ACTION_NONE:
            ar.amount = 0;
            ar.action = action;
            break;
        
        case ACTION_CHECK:
            ar.amount = 0;
            ar.action = action;
            break;

        case ACTION_CALL:
            if(currentBet >= chips) {
                ar.amount = chips;
                ar.action = ACTION_ALL_IN;
            } else {
                ar.action = action;
                ar.amount = BOT_BET_AMT;
            }
            break;

        case ACTION_BET:
            ar.amount = BOT_BET_AMT;
            ar.action = action;
            break;
        
        case ACTION_RAISE:
            if(currentBet >= chips) {
                ar.amount = chips;
                ar.action = ACTION_ALL_IN;
            } else {
                ar.action = action;
                ar.amount = BOT_BET_AMT;
            }
            break;
        
        case ACTION_FOLD:
            ar.amount = 0;
            ar.action = action;
            break;
        
        case ACTION_ALL_IN:
            ar.amount = chips;
            ar.action = action;
            break;

        default:
            ar.action = ACTION_NONE;
            ar.amount = 0;
    }

    return ar;
}


/*
// main for testing bot actions 
int main() {
    Player bot1 = {
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
        20,
        0,
        0
    };

    Player bot2 = {
        "Bot2",
        {
            {
                HEARTS,
                KING,
            },
            {
                HEARTS,
                JACK,
            }
        },
        STARTING_CHIPS,
        20,
        0,
        0
    };

    GameState gs;
    Card c1 = {DIAMONDS, ANT};
    Card c2 = {SPADES, ANT};
    Card c3 = {CLUBS, ANT};
    Card c4 = {HEARTS, TWO};
    Card c5 = {HEARTS, THREE};
    gs.communityCards[0] = c1;
    gs.communityCards[1] = c2;
    gs.communityCards[2] = c3;
    gs.communityCards[3] = c4;
    gs.communityCards[4] = c5;

    gs.currentBet = BOT_BET_AMT;
    gs.communityCardCount = 5;
    ActionRequest ar1 = botAction(&bot1, &gs);
    ActionRequest ar2 = botAction(&bot2, &gs);
    printf("ar1: %d %d\n", ar1.action, ar1.amount);
    printf("ar2: %d %d\n", ar2.action, ar2.amount);
}
*/
