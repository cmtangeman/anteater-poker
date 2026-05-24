#include "types.h"
#include "deck.h"
#include <stdlib.h>  // srand, rand
#include <time.h>    // time
#include <stdio.h>

void initializeDeck(Deck *deck) { 
	int index = 0;
	for(int suit = 0; suit < 4; suit++) { // Suits
		for(int rank = 0; rank < 15; rank++) { // Ranks
			deck->cards[index].suit = suit;
			deck->cards[index].rank = rank;
			index++;
		}
	}
	deck->size = MAX_DECK_SIZE;
	deck->top = 0;
}

void shuffleDeck(Deck *deck){
	srand(time(NULL));
	for(int i = 0; i < deck->size; i++){
		int random = rand() % (i + 1);
		Card temp = deck->cards[i]; // Grab bottom card
		deck->cards[i] = deck->cards[random]; // Swap bottom card with random card
		deck->cards[random] = temp; // Finish swap
	}
}

void resetDeck(Deck *deck) {
	initializeDeck(deck); // Rebuild the deck
	shuffleDeck(deck); // Shuffle the deck
}

Card drawCard(Deck *deck) {
	// No more cards left
	if(deck->top >= deck->size) {
		Card emptyCard;
		emptyCard.suit = -1;
		emptyCard.rank = -1;
		return emptyCard;
	}
	// Grab the next card
	Card card = deck->cards[deck->top];
	// Move top pointer forward
	deck->top++;
	return card;
}

void printCard(Card card) {
	// Suit names
	const char *suits[] = {
		"Hearts",
	       	"Diamonds",
	       	"Clubs",
	       	"Spades"
	};

	// Rank names
	const char *ranks[] = {
		"Ant",
	       	"Two",
	       	"Three",
	       	"Four",
	       	"Five",
	       	"Six",
		"Seven",
	       	"Eight",
	       	"Nine",
	       	"Ten",
	       	"Jack",
	       	"Queen",
		"King",
	       	"Ace",
	       	"Anteater"
	};

	// Invalid card check
	if(card.suit < 0 || card.suit > 3 || card.rank < 0 || card.rank > 14) {
		printf("Invalid card\n");
		return;
	}
	printf("%s of %s\n", ranks[card.rank], suits[card.suit]);
}

void printDeck(Deck *deck) {
	// Print remaining cards in the deck
	for(int i = deck->top; i < deck->size; i++) {
		printCard(deck->cards[i]);
	}
}
	

