#include "types.h"


Card cards[60];

void initializeDeck(Deck *deck) { 
	for(int i = 0; i < 4; i++){ // Suits
		for(int j = 0; j < 15; j++) // Ranks
deck->cards[i * 15 + j] = (Card){i, j};

}
deck->size = 60;
deck->top = 0;
}

Deck unshuffledDeck = {cards, 0, 60};

void shuffleDeck(Deck *deck){
	srand(time(NULL));
	for(int i = 0; i < deck->size; i++){
		int random = rand() % (i + 1);
		Card temp = deck->cards[i]; // Grab bottom card
		deck->cards[i] = deck->cards[random]; // Swap bottom card with random card
		deck->cards[random] = temp; // Finish swap
	}
}


	

