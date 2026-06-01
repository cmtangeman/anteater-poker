// Name: Abdulwahab Aldeyyeain
// Course: EECS 22L
// Assignment: Anteater Poker Project
// Description: Header file for poker hand rules.
// Checks poker hands and Anteater Poker special rules.

#ifndef RULES_H
#define RULES_H

#include "types.h"

// precondition: take in one card
// postcondition: return card value as an integer
int getCardValue(Card card);

// precondition: take in hand and size
// postcondition: return 1 if pair exists
int checkPair(Card hand[], int size);

// precondition: take in hand and size
// postcondition: return 1 if two pair exists
int checkTwoPair(Card hand[], int size);

// precondition: take in hand and size
// postcondition: return 1 if three of a kind exists
int checkThreeOfKind(Card hand[], int size);

// precondition: take in hand and size
// postcondition: return 1 if straight exists
int checkStraight(Card hand[], int size);

// precondition: take in hand and size
// postcondition: return 1 if flush exists
int checkFlush(Card hand[], int size);

// precondition: take in hand and size
// postcondition: return 1 if full house exists
int checkFullHouse(Card hand[], int size);

// precondition: take in hand and size
// postcondition: return 1 if four of a kind exists
int checkFourOfKind(Card hand[], int size);

// precondition: take in hand and size
// postcondition: return 1 if straight flush exists
int checkStraightFlush(Card hand[], int size);

// precondition: take in hand and size
// postcondition: return 1 if Anteater Feast exists
int checkAnteaterFeast(Card hand[], int size);

// precondition: take in hand and size
// postcondition: return ranking of the hand
HandRank evaluateHand(Card hand[], int size);

#endif