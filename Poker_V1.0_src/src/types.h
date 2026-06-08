#ifndef TYPES_H
#define TYPES_H

//Global constants

#define MAX_PLAYERS 6
#define MAX_USERNAME_LEN 50

#define HAND_SIZE 2
#define COMMUNITY_CARDS 5
#define MAX_DECK_SIZE 60

#define MAX_RANK_LEN 20
#define MAX_SUIT_LEN 20

#define STARTING_CHIPS 20
#define MIN_BET 1


// Card-related enums

typedef enum {
    HEARTS,
    DIAMONDS,
    CLUBS,
    SPADES
} Suit;

typedef enum {
    ANT,	
    TWO,
    THREE,
    FOUR,
    FIVE,
    SIX,
    SEVEN,
    EIGHT,
    NINE,
    TEN,
    JACK,
    QUEEN,
    KING,
    ACE,
    Anteater
} Rank;

// Game state enums

typedef enum {
    GAME_WAITING,
    GAME_PREFLOP,
    GAME_FLOP,
    GAME_TURN,
    GAME_RIVER,
    GAME_SHOWDOWN,
    GAME_OVER
} GamePhase;

typedef enum {
    ACTION_NONE,
    ACTION_CHECK,
    ACTION_CALL,
    ACTION_BET,
    ACTION_RAISE,
    ACTION_FOLD,
    ACTION_ALL_IN
} PlayerAction;

typedef enum {
    HUMAN_PLAYER,
    BOT_PLAYER
} PlayerType;

// Hand ranking enum

typedef enum {
    HIGH_CARD,
    ONE_PAIR,
    TWO_PAIR,
    THREE_OF_A_KIND,
    STRAIGHT,
    FLUSH,
    FULL_HOUSE,
    FOUR_OF_A_KIND,
    STRAIGHT_FLUSH,
    ANTEATER_FEAST
} HandRank;

//
  // Structs

typedef struct {
    Suit suit;
    Rank rank;
} Card;

typedef struct {
    Card cards[MAX_DECK_SIZE];
    int top;
    int size;
} Deck;

typedef struct {
    char username[MAX_USERNAME_LEN];

    Card hand[HAND_SIZE];

    int ready;
    int chips;
    int currentBet;
    int folded;
    int allIn;
    int seat;

    PlayerType type;
} Player;

typedef struct {
    Player players[MAX_PLAYERS];
    int playerCount;

    Card communityCards[COMMUNITY_CARDS];
    int communityCardCount;

    Deck deck;

    int pot;
    int currentBet;
    int dealerIndex;
    int currentTurn;

    GamePhase phase;
} GameState;

typedef struct {
    HandRank rank;
    Card bestCards[5];
    int score;
} HandResult;

typedef struct {
    PlayerAction action;
    int amount;
} ActionRequest;

#endif
