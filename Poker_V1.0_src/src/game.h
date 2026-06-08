// Name: Abdulwahab Aldeyyeain
// Course: EECS 22L
// Assignment: Anteater Poker Project
// Description: Header file for poker game logic.
// Controls game setup, turns, betting, cards, and round flow.


#ifndef GAME_H
#define GAME_H


#include "types.h"


// precondition: take in game state
// postcondition: game state is initialized
void initializeGame(GameState *gameState);


// precondition: take in game state
// postcondition: all players are given starting chips
void initializePlayers(GameState *gameState);


// precondition: take in game state
// postcondition: new round is started
void startRound(GameState *gameState);


// precondition: take in game state
// postcondition: all player bets are reset
void resetBets(GameState *gameState);


// precondition: take in game state
// postcondition: one card is returned from deck
Card drawCard(GameState *gameState);


// precondition: take in game state
// postcondition: each player gets two cards
void dealPlayerCards(GameState *gameState);


// precondition: take in game state
// postcondition: three community cards are dealt
void dealFlop(GameState *gameState);


// precondition: take in game state
// postcondition: one community card is dealt
void dealTurn(GameState *gameState);


// precondition: take in game state
// postcondition: one community card is dealt
void dealRiver(GameState *gameState);


// precondition: take in game state
// postcondition: move to next player turn
void nextTurn(GameState *gameState);


// precondition: take in game state and player index
// postcondition: return 1 if player can act
int canPlayerAct(GameState *gameState, int playerIndex);


// precondition: take in game state and player index
// postcondition: return player hand score
int getPlayerScore(GameState *gameState, int playerIndex);


// precondition: take in game state and player index
// postcondition: return 1 if player can check
int canCheck(GameState *gameState, int playerIndex);


// precondition: take in game state, player index, and amount
// postcondition: return 1 if bet is valid
int canBet(GameState *gameState, int playerIndex, int amount);


// precondition: take in game state, player index, and amount
// postcondition: return 1 if raise is valid
int canRaise(GameState *gameState, int playerIndex, int amount);


// precondition: take in game state, player index, and amount
// postcondition: player bet is processed
void processBet(GameState *gameState, int playerIndex, int amount);


// precondition: take in game state and player index
// postcondition: player calls current bet
void callBet(GameState *gameState, int playerIndex);


// precondition: take in game state and player index
// postcondition: player checks
void checkPlayer(GameState *gameState, int playerIndex);


// precondition: take in game state and player index
// postcondition: player folds
void foldPlayer(GameState *gameState, int playerIndex);


// precondition: take in game state and player index
// postcondition: player goes all in
void allInPlayer(GameState *gameState, int playerIndex);


// precondition: take in game state and action request
// postcondition: action is applied to game
void processAction(GameState *gameState,
                  int playerIndex,
                  ActionRequest request);


// precondition: take in game state
// postcondition: move to next game phase
void advancePhase(GameState *gameState);


// precondition: take in game state
// postcondition: return number of active players
int countActivePlayers(GameState *gameState);


// precondition: take in game state
// postcondition: return winner index
int determineWinner(GameState *gameState);


// precondition: take in game state and winner index
// postcondition: winner receives pot
void endRound(GameState *gameState, int winnerIndex);


#endif