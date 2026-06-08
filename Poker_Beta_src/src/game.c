// Name: Abdulwahab Aldeyyeain
// Course: EECS 22L
// Assignment: Anteater Poker Project
// Description: Implementation of poker game logic.
// Controls turns, betting, dealing, phases, and winner selection.


#include "game.h"
#include "deck.h"
#include "rules.h"
#include <stdio.h>
#include <string.h>


// precondition: take in game state
// postcondition: game state is initialized
void initializeGame(GameState *gameState) {


   gameState->playerCount = MAX_PLAYERS;
   gameState->pot = 0;
   gameState->currentBet = 0;
   gameState->dealerIndex = 0;
   gameState->currentTurn = 0;
   gameState->communityCardCount = 0;
   gameState->phase = GAME_WAITING;


   initializePlayers(gameState);
   initializeDeck(&gameState->deck);
}


// precondition: take in game state
// postcondition: all players are given starting chips
void initializePlayers(GameState *gameState) {


   int i;


   for (i = 0; i < MAX_PLAYERS; i++) {


       gameState->players[i].chips = STARTING_CHIPS;
       gameState->players[i].currentBet = 0;
       gameState->players[i].folded = 0;
       gameState->players[i].allIn = 0;
       gameState->players[i].ready = 0;
       gameState->players[i].type = HUMAN_PLAYER;
   }
}


// precondition: take in game state
// postcondition: new round is started
void startRound(GameState *gameState) {


   int i;


   gameState->pot = 0;
   gameState->currentBet = 0;
   gameState->communityCardCount = 0;
   gameState->phase = GAME_PREFLOP;


   initializeDeck(&gameState->deck);
   shuffleDeck(&gameState->deck);


   for (i = 0; i < MAX_PLAYERS; i++) {


       gameState->players[i].folded = 0;
       gameState->players[i].allIn = 0;
       gameState->players[i].currentBet = 0;
   }


   dealPlayerCards(gameState);


   // gameState->currentTurn = gameState->dealerIndex + 1;


   if (gameState->currentTurn >= gameState->playerCount) {
       gameState->currentTurn = 0;
   }


   if (canPlayerAct(gameState, gameState->currentTurn) == 0) {
       nextTurn(gameState);
   }
}


// precondition: take in game state
// postcondition: all player bets are reset
void resetBets(GameState *gameState) {


   int i;


   gameState->currentBet = 0;


   for (i = 0; i < MAX_PLAYERS; i++) {
       gameState->players[i].currentBet = 0;
   }
}


// precondition: take in game state
// postcondition: one card is returned from deck
Card drawCard(GameState *gameState) {


   Card card;


   if (gameState->deck.top < gameState->deck.size) {


       card = gameState->deck.cards[gameState->deck.top];


       gameState->deck.top++;
   }
   else {


       card.rank = ANT;
       card.suit = HEARTS;


       printf("Deck is empty.\n");
   }


   return card;
}


// precondition: take in game state
// postcondition: each player gets two cards
void dealPlayerCards(GameState *gameState) {


   int i;
   int j;


   for (i = 0; i < gameState->playerCount; i++) {


       for (j = 0; j < HAND_SIZE; j++) {
           gameState->players[i].hand[j] = drawCard(gameState);
       }
   }
}


// precondition: take in game state
// postcondition: three community cards are dealt
void dealFlop(GameState *gameState) {


   int i;


   for (i = 0; i < 3; i++) {


       if (gameState->communityCardCount < COMMUNITY_CARDS) {


           gameState->communityCards[gameState->communityCardCount] =
               drawCard(gameState);


           gameState->communityCardCount++;
       }
   }
}


// precondition: take in game state
// postcondition: one community card is dealt
void dealTurn(GameState *gameState) {


   if (gameState->communityCardCount < COMMUNITY_CARDS) {


       gameState->communityCards[gameState->communityCardCount] =
           drawCard(gameState);


       gameState->communityCardCount++;
   }
}


// precondition: take in game state
// postcondition: one community card is dealt
void dealRiver(GameState *gameState) {


   if (gameState->communityCardCount < COMMUNITY_CARDS) {


       gameState->communityCards[gameState->communityCardCount] =
           drawCard(gameState);


       gameState->communityCardCount++;
   }
}


// precondition: take in game state
// postcondition: move to next player turn
void nextTurn(GameState *gameState) {


   int checkedPlayers = 0;


   do {
       gameState->currentTurn++;


       if (gameState->currentTurn >= gameState->playerCount) {
           gameState->currentTurn = 0;
       }


       checkedPlayers++;


   } while (canPlayerAct(gameState, gameState->currentTurn) == 0 &&
            checkedPlayers < gameState->playerCount);
}


// precondition: take in game state and player index
// postcondition: return 1 if player can act
int canPlayerAct(GameState *gameState, int playerIndex) {


   if (playerIndex < 0 || playerIndex >= gameState->playerCount) {
       return 0;
   }
   else if (gameState->players[playerIndex].folded == 1) {
       return 0;
   }
   else if (gameState->players[playerIndex].allIn == 1) {
       return 0;
   }
   else if (gameState->players[playerIndex].chips <= 0) {
       return 0;
   }
   else {
       return 1;
   }
}


// precondition: take in game state and player index
// postcondition: return player hand score
int getPlayerScore(GameState *gameState, int playerIndex) {


   int j;
   int index;


   Card fullHand[7];


   fullHand[0] = gameState->players[playerIndex].hand[0];
   fullHand[1] = gameState->players[playerIndex].hand[1];


   index = 2;


   for (j = 0; j < gameState->communityCardCount; j++) {
       fullHand[index] = gameState->communityCards[j];
       index++;
   }


   return getHandScore(fullHand, index);
}


// precondition: take in game state and player index
// postcondition: return 1 if player can check
int canCheck(GameState *gameState, int playerIndex) {


   if (gameState->players[playerIndex].currentBet ==
       gameState->currentBet) {


       return 1;
   }
   else {
       return 0;
   }
}


// precondition: take in game state, player index, and amount
// postcondition: return 1 if bet is valid
int canBet(GameState *gameState, int playerIndex, int amount) {


   if (amount <= 0) {
       return 0;
   }
   else if (gameState->players[playerIndex].chips < amount) {
       return 0;
   }
   else {
       return 1;
   }
}


// precondition: take in game state, player index, and amount
// postcondition: return 1 if raise is valid
int canRaise(GameState *gameState, int playerIndex, int amount) {


   if (canBet(gameState, playerIndex, amount) == 0) {
       return 0;
   }
   else if (gameState->currentBet > 0 &&
            amount < gameState->currentBet) {
       return 0;
   }
   else {
       return 1;
   }
}


// precondition: take in game state, player index, and amount
// postcondition: player bet is processed
void processBet(GameState *gameState, int playerIndex, int amount) {


   if (amount <= 0) {
       printf("Invalid bet amount.\n");
   }
   else if (gameState->players[playerIndex].chips < amount) {
       printf("Player does not have enough chips.\n");
   }
   else {
       gameState->players[playerIndex].chips =
           gameState->players[playerIndex].chips - amount;


       gameState->players[playerIndex].currentBet =
           gameState->players[playerIndex].currentBet + amount;


       gameState->pot = gameState->pot + amount;


       if (gameState->players[playerIndex].currentBet >
           gameState->currentBet) {


           gameState->currentBet =
               gameState->players[playerIndex].currentBet;
       }


       if (gameState->players[playerIndex].chips == 0) {
           gameState->players[playerIndex].allIn = 1;
       }
   }
}


// precondition: take in game state and player index
// postcondition: player calls current bet
void callBet(GameState *gameState, int playerIndex) {


   int amountToCall;


   amountToCall = gameState->currentBet -
                  gameState->players[playerIndex].currentBet;


   if (amountToCall <= 0) {
       checkPlayer(gameState, playerIndex);
   }
   else if (amountToCall >= gameState->players[playerIndex].chips) {
       allInPlayer(gameState, playerIndex);
   }
   else {
       processBet(gameState, playerIndex, amountToCall);
   }
}


// precondition: take in game state and player index
// postcondition: player checks
void checkPlayer(GameState *gameState, int playerIndex) {


   if (gameState->players[playerIndex].currentBet ==
       gameState->currentBet) {


       printf(" %s  checked.\n", gameState->players[playerIndex].username);
   }
   else {
       printf("Player cannot check.\n");
   }
}


// precondition: take in game state and player index
// postcondition: player folds
void foldPlayer(GameState *gameState, int playerIndex) {


   gameState->players[playerIndex].folded = 1;
   printf(" %s  folded.\n", gameState->players[playerIndex].username);
}


// precondition: take in game state and player index
// postcondition: player goes all in
void allInPlayer(GameState *gameState, int playerIndex) {


   int amount;


   amount = gameState->players[playerIndex].chips;


   if (amount > 0) {
       processBet(gameState, playerIndex, amount);
       gameState->players[playerIndex].allIn = 1;
       printf(" %s  is all in.\n", gameState->players[playerIndex].username);
   }
}


// precondition: take in game state and action request
// postcondition: action is applied to game
void processAction(GameState *gameState,
                  int playerIndex,
                  ActionRequest request) {


   int validAction = 1;


   if (request.action == ACTION_CHECK) {


       if (canCheck(gameState, playerIndex) == 1) {
           checkPlayer(gameState, playerIndex);
       }
       else {
           printf("Player cannot check.\n");
           validAction = 0;
       }
   }
   else if (request.action == ACTION_CALL) {
       callBet(gameState, playerIndex);
   }
   else if (request.action == ACTION_BET) {


       if (canBet(gameState, playerIndex, request.amount) == 1) {
           processBet(gameState, playerIndex, request.amount);
       }
       else {
           printf("Invalid bet.\n");
           validAction = 0;
       }
   }
   else if (request.action == ACTION_RAISE) {


       if (canRaise(gameState, playerIndex, request.amount) == 1) {
           processBet(gameState, playerIndex, request.amount);
       }
       else {
           printf("Invalid raise.\n");
           validAction = 0;
       }
   }
   else if (request.action == ACTION_FOLD) {
       foldPlayer(gameState, playerIndex);
   }
   else if (request.action == ACTION_ALL_IN) {
       allInPlayer(gameState, playerIndex);
   }
   else {
       printf("No action taken.\n");
       validAction = 0;
   }


   if (validAction == 1) {
       nextTurn(gameState);
   }
}


// precondition: take in game state
// postcondition: move to next game phase
void advancePhase(GameState *gameState) {


   resetBets(gameState);


   if (gameState->phase == GAME_PREFLOP) {
       dealFlop(gameState);
       gameState->phase = GAME_FLOP;
   }
   else if (gameState->phase == GAME_FLOP) {
       dealTurn(gameState);
       gameState->phase = GAME_TURN;
   }
   else if (gameState->phase == GAME_TURN) {
       dealRiver(gameState);
       gameState->phase = GAME_RIVER;
   }
   else if (gameState->phase == GAME_RIVER) {
       gameState->phase = GAME_SHOWDOWN;
   }
   else {
       gameState->phase = GAME_OVER;
   }


   gameState->currentTurn = gameState->dealerIndex + 1;


   if (gameState->currentTurn >= gameState->playerCount) {
       gameState->currentTurn = 0;
   }


   if (canPlayerAct(gameState, gameState->currentTurn) == 0) {
       nextTurn(gameState);
   }
}


// precondition: take in game state
// postcondition: return number of active players
int countActivePlayers(GameState *gameState) {


   int i;
   int count = 0;


   for (i = 0; i < gameState->playerCount; i++) {


       if (gameState->players[i].folded == 0) {
           count++;
       }
   }


   return count;
}


// precondition: take in game state
// postcondition: return winner index
int determineWinner(GameState *gameState) {


   int i;
   int winnerIndex = -1;


   int bestScore = -1;
   int currentScore;


   for (i = 0; i < gameState->playerCount; i++) {


       if (gameState->players[i].folded == 0) {


           currentScore = getPlayerScore(gameState, i);


           if (winnerIndex == -1) {
               winnerIndex = i;
               bestScore = currentScore;
           }
           else if (currentScore > bestScore) {
               winnerIndex = i;
               bestScore = currentScore;
           }
       }
   }


   return winnerIndex;
}


// precondition: take in game state and winner index
// postcondition: winner receives pot
void endRound(GameState *gameState, int winnerIndex) {


   int i;
   int winnerScore;
   int currentScore;
   int tieCount = 0;
   int splitAmount;
   int remainder;


   if (winnerIndex >= 0 && winnerIndex < gameState->playerCount) {


       winnerScore = getPlayerScore(gameState, winnerIndex);


       for (i = 0; i < gameState->playerCount; i++) {


           if (gameState->players[i].folded == 0) {


               currentScore = getPlayerScore(gameState, i);


               if (currentScore == winnerScore) {
                   tieCount++;
               }
           }
       }


       if (tieCount > 0) {


           splitAmount = gameState->pot / tieCount;
           remainder = gameState->pot % tieCount;


           for (i = 0; i < gameState->playerCount; i++) {


               if (gameState->players[i].folded == 0) {


                   currentScore = getPlayerScore(gameState, i);


                   if (currentScore == winnerScore) {
                       gameState->players[i].chips =
                           gameState->players[i].chips + splitAmount;
                   }
               }
           }


           gameState->players[winnerIndex].chips =
               gameState->players[winnerIndex].chips + remainder;
       }


       gameState->pot = 0;


       gameState->dealerIndex++;


       if (gameState->dealerIndex >= gameState->playerCount) {
           gameState->dealerIndex = 0;
       }


       gameState->phase = GAME_OVER;
   }
}