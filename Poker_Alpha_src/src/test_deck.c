#include <stdio.h>
#include "types.h"
#include "deck.h"

void printDeck(Deck *deck) {
    for (int i = 0; i < deck->size; i++) {
        printf("Card %d: Suit %d, Rank %d\n", i, deck->cards[i].suit, deck->cards[i].rank);
    }
}

int main() {
	Deck deck;

	
    // Test initializeDeck
    printf("Initialized Deck:-\n");
    initializeDeck(&deck);
    printDeck(&deck);

    // Test shuffleDeck
    printf("\nShuffled Deck\n");
    shuffleDeck(&deck);
    printDeck(&deck);

    // Test top
    printf("\nTop index: %d\n", deck.top);
    printf("Size: %d\n", deck.size);

    return 0;
}
