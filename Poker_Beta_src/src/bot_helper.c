// Program file that serves helper functions for the bot.c file

#include "bot_helper.h"

// card equivalence fn
int card_eq(Card a, Card b) {
    return a.rank == b.rank && a.suit == b.suit;
}

// deck builder fn that builds a deck with cards except for the current hole
// and community cards
Deck build_remaining_deck(Card *my_holes, int num_holes,Card *community, 
    int num_community) {

    Deck d;
    int rank, suit;

    d.size = 0;

    // Build full deck
    for (suit = 0; suit < 4; suit++) {
        for (rank = 0; rank <= 14; rank++) {
            Card c = {rank, suit};

            // Skip cards we already know about
            int known = 0;
            for (int i = 0; i < num_holes; i++)
                if (card_eq(c, my_holes[i])) { known = 1; break; }
            for (int i = 0; i < num_community; i++)
                if (card_eq(c, community[i])) { known = 1; break; }

            if (!known)
                d.cards[d.size++] = c;
        }
    }
    return d;
}


// shuffle fn that rips off of fisher yates
void shuffle(Deck *d) {
    for (int i = d->size - 1; i > 0; i--) {
        int j = rand() % (i + 1);
        Card tmp    = d->cards[i];
        d->cards[i] = d->cards[j];
        d->cards[j] = tmp;
    }
}

// fn that fills in the rest of the community cards
void complete_board(Card *community, int num_community,
                    Deck *d,         Card *out_board) {
    // Copy known community cards
    for (int i = 0; i < num_community; i++)
        out_board[i] = community[i];

    // Deal remaining cards from shuffled deck
    int deck_idx = 0;
    for (int i = num_community; i < 5; i++)
        out_board[i] = d->cards[deck_idx++];
}

// fn that deals the opponent hole cards
void deal_opponent(Deck *d, int deck_offset, Card *opp_holes) {
    opp_holes[0] = d->cards[deck_offset];
    opp_holes[1] = d->cards[deck_offset + 1];
}

// Combine hole cards + community cards into one array and call evaluateHand
HandRank evaluate_with_known(Card *hole_cards, Card *board, int board_size) {
    Card combined[7];
    combined[0] = hole_cards[0];
    combined[1] = hole_cards[1];
    for (int i = 0; i < board_size; i++)
        combined[2 + i] = board[i];

    return evaluateHand(combined, 2 + board_size);
}

// main helper fn that utilizes all the above fns to do a monte carlo simulation
EquityResult monte_carlo(Card *my_holes, int num_holes,
    Card *community, int num_community,
    int   num_opponents, int   num_simulations) {

    Deck deck = build_remaining_deck(my_holes, num_holes, community, num_community);

    int wins = 0, ties = 0, losses = 0;
    int cards_needed = (5 - num_community) + (2 * num_opponents);

    for (int sim = 0; sim < num_simulations; sim++) {
        shuffle(&deck);

        // Safety check to ensure enough cards to run this simulation
        if (deck.size < cards_needed) break;

        // Complete the community board
        Card board[5];
        complete_board(community, num_community, &deck, board);

        int deck_offset = 5 - num_community;

        // Evaluate my hand
        HandRank my_rank = evaluate_with_known(my_holes,  board, 5);

        // Evaluate all opponents, take their best hand
        int i_win = 1;
        int i_tie = 0;

        for (int opp = 0; opp < num_opponents; opp++) {
            Card opp_holes[2];
            deal_opponent(&deck, deck_offset + (opp * 2), opp_holes);

            // evaluate my opponents hand
            HandRank opp_rank = evaluate_with_known(opp_holes, board, 5);

            if (opp_rank > my_rank) { 
                i_win = 0; 
                i_tie = 0; 
                break; 
            }

            if (opp_rank == my_rank)  
                i_tie = 1;
        }

        if (i_win) {
            wins++;
        } else if (i_tie) { 
            ties++; 
        } else {
            losses++;
        }
    }

    return (EquityResult){
        .win_probability  = (float)wins   / num_simulations,
        .tie_probability  = (float)ties   / num_simulations,
        .loss_probability = (float)losses / num_simulations
    };
}