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

    // return easyMode(bot, all_cards, card_count);
    return medMode(bot, bot->hand, gs->communityCards, gs->communityCardCount);
}


// Function that performs actions corresponding to the easy mode for the bot
ActionRequest easyMode(Player *bot, Card *all_cards, int card_count) {
    ActionRequest ar;

    // go all in for strongest hand value
    if(evaluateHand(all_cards, card_count) == 10) {
        // bot->currentBet = bot->chips;
        
        ar.action = ACTION_ALL_IN;

        bot->allIn = 1;
    }
    // if hand is strong, increase the bet amount
    if(evaluateHand(all_cards, card_count) > 8) {
        // bot->currentBet = .8 * bot->chips;

        ar.action = ACTION_RAISE;
    
    // if hand value is mediocre, continue with the bet amount  
    } else if(evaluateHand(all_cards, card_count) > 4) {
        ar.action = ACTION_CALL;

    // if hand value is poor, fold
    } else {
        bot->folded = 1;
        ar.action = ACTION_FOLD;
    }

    ar.amount = bot->currentBet;

    return ar;
}


// Function that performs actions corresponding to the medium mode for the bot
ActionRequest medMode(Player *bot, Card *bot_cards, Card *comm_card, 
    int comm_card_count) {

    ActionRequest ar;
    int opps_no;

    // temp hard coded opp value
    opps_no = 1;

    // call the monte carlo function
    EquityResult eq = monte_carlo(bot_cards, HAND_SIZE, comm_card, comm_card_count,
        opps_no, 10000);

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

    GameState gs;
    Card c = {HEARTS, ANT};
    gs.communityCards[0] = c;
    gs.communityCards[1] = c;
    gs.communityCards[2] = c;
    gs.communityCards[3] = c;
    gs.communityCards[4] = c;

    ActionRequest ar = botAction(&bot, &gs);
    printf("%d %d\n", ar.action, ar.amount);
}
*/