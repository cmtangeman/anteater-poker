#ifndef DECK_H
#define DECK_H

#include "types.h"

void initializeDeck(Deck *deck);
void shuffleDeck(Deck *deck);
void resetDeck(Deck *deck);

// Card drawCard(Deck *deck); Using game.c verion

// Help with debugging
void printCard(Card card);
void printDeck(Deck *deck);

#endif

