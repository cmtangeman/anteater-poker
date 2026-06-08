// Name: Abdulwahab Aldeyyeain
// Course: EECS 22L
// Assignment: Anteater Poker Project
// Description: Implementation of poker hand rules.
// Checks hand rankings and Anteater Poker special hand.


#include "rules.h"


// precondition: take in one card
// postcondition: return card value as an integer
int getCardValue(Card card) {


   if (card.rank == ANT) {
       return 1;
   }
   else if (card.rank == TWO) {
       return 2;
   }
   else if (card.rank == THREE) {
       return 3;
   }
   else if (card.rank == FOUR) {
       return 4;
   }
   else if (card.rank == FIVE) {
       return 5;
   }
   else if (card.rank == SIX) {
       return 6;
   }
   else if (card.rank == SEVEN) {
       return 7;
   }
   else if (card.rank == EIGHT) {
       return 8;
   }
   else if (card.rank == NINE) {
       return 9;
   }
   else if (card.rank == TEN) {
       return 10;
   }
   else if (card.rank == JACK) {
       return 11;
   }
   else if (card.rank == QUEEN) {
       return 12;
   }
   else if (card.rank == KING) {
       return 13;
   }
   else if (card.rank == ACE) {
       return 14;
   }
   else if (card.rank == Anteater) {
       return 13;
   }
   else {
       return 0;
   }
}


// precondition: take in hand and size
// postcondition: return highest card value
int getHighestCardValue(Card hand[], int size) {


   int i;
   int value;
   int highest = 0;


   for (i = 0; i < size; i++) {


       value = getCardValue(hand[i]);


       if (value > highest) {
           highest = value;
       }
   }


   return highest;
}


// precondition: take in hand and size
// postcondition: return strongest duplicate value
int getBestDuplicateValue(Card hand[], int size, int neededCount) {


   int values[15] = {0};
   int i;
   int value;
   int bestValue = 0;


   for (i = 0; i < size; i++) {


       value = getCardValue(hand[i]);


       if (value >= 1 && value <= 14) {
           values[value]++;
       }
   }


   for (i = 14; i >= 1; i--) {


       if (values[i] >= neededCount) {
           bestValue = i;
           break;
       }
   }


   return bestValue;
}


// precondition: take in hand and size
// postcondition: return straight high card value
int getStraightHighValue(Card hand[], int size) {


   int values[15] = {0};
   int i;
   int value;
   int count = 0;
   int highValue = 0;


   for (i = 0; i < size; i++) {


       value = getCardValue(hand[i]);


       if (value >= 1 && value <= 14) {
           values[value] = 1;
       }
   }


   if (values[14] == 1) {
       values[1] = 1;
   }


   for (i = 1; i <= 14; i++) {


       if (values[i] == 1) {
           count++;
       }
       else {
           count = 0;
       }


       if (count >= 5) {
           highValue = i;
       }
   }


   return highValue;
}


// precondition: take in hand and size
// postcondition: return hand score
int getHandScore(Card hand[], int size) {


   HandRank rank;
   int value;
   int score;


   rank = evaluateHand(hand, size);
   value = 0;


   if (rank == ANTEATER_FEAST) {
       value = 100;
   }
   else if (rank == STRAIGHT_FLUSH) {
       value = getStraightHighValue(hand, size);
   }
   else if (rank == FOUR_OF_A_KIND) {
       value = getBestDuplicateValue(hand, size, 4);
   }
   else if (rank == FULL_HOUSE) {
       value = getBestDuplicateValue(hand, size, 3);
   }
   else if (rank == FLUSH) {
       value = getHighestCardValue(hand, size);
   }
   else if (rank == STRAIGHT) {
       value = getStraightHighValue(hand, size);
   }
   else if (rank == THREE_OF_A_KIND) {
       value = getBestDuplicateValue(hand, size, 3);
   }
   else if (rank == TWO_PAIR) {
       value = getBestDuplicateValue(hand, size, 2);
   }
   else if (rank == ONE_PAIR) {
       value = getBestDuplicateValue(hand, size, 2);
   }
   else {
       value = getHighestCardValue(hand, size);
   }


   score = rank * 100 + value;


   return score;
}


// precondition: take in hand and size
// postcondition: return 1 if pair exists
int checkPair(Card hand[], int size) {


   int i;
   int j;


   for (i = 0; i < size; i++) {


       for (j = i + 1; j < size; j++) {


           if (getCardValue(hand[i]) == getCardValue(hand[j])) {
               return 1;
           }
       }
   }


   return 0;
}


// precondition: take in hand and size
// postcondition: return 1 if two pair exists
int checkTwoPair(Card hand[], int size) {


   int i;
   int j;
   int pairCount = 0;
   int usedValues[15] = {0};
   int value;


   for (i = 0; i < size; i++) {


       value = getCardValue(hand[i]);


       if (value >= 1 && value <= 14) {


           if (usedValues[value] == 0) {


               for (j = i + 1; j < size; j++) {


                   if (value == getCardValue(hand[j])) {
                       pairCount++;
                       usedValues[value] = 1;
                       break;
                   }
               }
           }
       }
   }


   if (pairCount >= 2) {
       return 1;
   }
   else {
       return 0;
   }
}


// precondition: take in hand and size
// postcondition: return 1 if three of a kind exists
int checkThreeOfKind(Card hand[], int size) {


   int i;
   int j;
   int count;


   for (i = 0; i < size; i++) {


       count = 1;


       for (j = 0; j < size; j++) {


           if (i != j &&
               getCardValue(hand[i]) == getCardValue(hand[j])) {
               count++;
           }
       }


       if (count >= 3) {
           return 1;
       }
   }


   return 0;
}


// precondition: take in hand and size
// postcondition: return 1 if straight exists
int checkStraight(Card hand[], int size) {


   int values[15] = {0};
   int i;
   int value;
   int count = 0;


   for (i = 0; i < size; i++) {


       value = getCardValue(hand[i]);


       if (value >= 1 && value <= 14) {
           values[value] = 1;
       }
   }


   if (values[14] == 1) {
       values[1] = 1;
   }


   for (i = 1; i <= 14; i++) {


       if (values[i] == 1) {
           count++;
       }
       else {
           count = 0;
       }


       if (count >= 5) {
           return 1;
       }
   }


   return 0;
}


// precondition: take in hand and size
// postcondition: return 1 if flush exists
int checkFlush(Card hand[], int size) {


   int hearts = 0;
   int diamonds = 0;
   int clubs = 0;
   int spades = 0;
   int i;


   for (i = 0; i < size; i++) {


       if (hand[i].suit == HEARTS) {
           hearts++;
       }
       else if (hand[i].suit == DIAMONDS) {
           diamonds++;
       }
       else if (hand[i].suit == CLUBS) {
           clubs++;
       }
       else if (hand[i].suit == SPADES) {
           spades++;
       }
   }


   if (hearts >= 5 || diamonds >= 5 || clubs >= 5 || spades >= 5) {
       return 1;
   }
   else {
       return 0;
   }
}


// precondition: take in hand and size
// postcondition: return 1 if full house exists
int checkFullHouse(Card hand[], int size) {


   int values[15] = {0};
   int i;
   int value;
   int threeFound = 0;
   int pairFound = 0;


   for (i = 0; i < size; i++) {


       value = getCardValue(hand[i]);


       if (value >= 1 && value <= 14) {
           values[value]++;
       }
   }


   for (i = 1; i <= 14; i++) {


       if (values[i] >= 3 && threeFound == 0) {
           threeFound = 1;
       }
       else if (values[i] >= 2) {
           pairFound = 1;
       }
   }


   if (threeFound == 1 && pairFound == 1) {
       return 1;
   }
   else {
       return 0;
   }
}


// precondition: take in hand and size
// postcondition: return 1 if four of a kind exists
int checkFourOfKind(Card hand[], int size) {


   int values[15] = {0};
   int i;
   int value;


   for (i = 0; i < size; i++) {


       value = getCardValue(hand[i]);


       if (value >= 1 && value <= 14) {
           values[value]++;
       }
   }


   for (i = 1; i <= 14; i++) {


       if (values[i] >= 4) {
           return 1;
       }
   }


   return 0;
}


// precondition: take in hand and size
// postcondition: return 1 if straight flush exists
int checkStraightFlush(Card hand[], int size) {


   int i;
   int j;
   int tempSize;
   Card tempHand[7];


   for (i = HEARTS; i <= SPADES; i++) {


       tempSize = 0;


       for (j = 0; j < size; j++) {


           if (hand[j].suit == (Suit)i) {
               tempHand[tempSize] = hand[j];
               tempSize++;
           }
       }


       if (tempSize >= 5) {


           if (checkStraight(tempHand, tempSize) == 1) {
               return 1;
           }
       }
   }


   return 0;
}


// precondition: take in hand and size
// postcondition: return 1 if Anteater Feast exists
int checkAnteaterFeast(Card hand[], int size) {


   int anteaterFound = 0;
   int antCount = 0;
   int i;


   for (i = 0; i < size; i++) {


       if (hand[i].rank == Anteater) {
           anteaterFound = 1;
       }


       if (hand[i].rank == ANT) {
           antCount++;
       }
   }


   if (anteaterFound == 1 && antCount >= 4) {
       return 1;
   }
   else {
       return 0;
   }
}


// precondition: take in hand and size
// postcondition: return ranking of the hand
HandRank evaluateHand(Card hand[], int size) {


   if (checkAnteaterFeast(hand, size) == 1) {
       return ANTEATER_FEAST;
   }
   else if (checkStraightFlush(hand, size) == 1) {
       return STRAIGHT_FLUSH;
   }
   else if (checkFourOfKind(hand, size) == 1) {
       return FOUR_OF_A_KIND;
   }
   else if (checkFullHouse(hand, size) == 1) {
       return FULL_HOUSE;
   }
   else if (checkFlush(hand, size) == 1) {
       return FLUSH;
   }
   else if (checkStraight(hand, size) == 1) {
       return STRAIGHT;
   }
   else if (checkThreeOfKind(hand, size) == 1) {
       return THREE_OF_A_KIND;
   }
   else if (checkTwoPair(hand, size) == 1) {
       return TWO_PAIR;
   }
   else if (checkPair(hand, size) == 1) {
       return ONE_PAIR;
   }
   else {
       return HIGH_CARD;
   }
}