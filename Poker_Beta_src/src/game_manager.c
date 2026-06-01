#include "poker_server.h"
#include "types.h"
#include "game.h"
#include "rules.h"
#include "bot.h"


void printCard(Card c);
void printGameState(GameState *gameState);
void printPlayerHand(GameState *gs, int playerIndex);

void printPlayerHand(GameState *gs, int playerIndex) {
    printf("\nYour hand: ");
    printCard(gs->players[playerIndex].hand[0]);
    printf(", ");
    printCard(gs->players[playerIndex].hand[1]);
    printf("\nYour chips: $%d\n", gs->players[playerIndex].chips);
}

void printGameState(GameState *gameState) {
    printf("\n--- Community Cards ---\n");
    for (int i = 0; i < gameState->communityCardCount; i++) {
        printCard(gameState->communityCards[i]);
        printf("\n");
    }
    printf("Pot: %d chips\n", gameState->pot);
    printf("Current Bet: %d chips\n", gameState->currentBet);
    printf("\n--- Players ---\n");
    for (int i = 0; i < gameState->playerCount; i++) {
        printf("Player %d: %s - Chips: %d", i, gameState->players[i].username, gameState->players[i].chips);
        if (gameState->players[i].folded) printf(" [FOLDED]");
        if (gameState->players[i].allIn) printf(" [ALL IN]");
        printf("\n");
    }
}

// -> Eventually encapsulate main logic in here for local debugging void gameLoop(Player )

ActionRequest getUserMove(GameState *gameState, int playerIndex){
    ActionRequest request;
        int choice;
        int amount;

    printf("Your Turn!");
    printPlayerHand(gameState, playerIndex);
    printf("\n1) Check  2) Call  3) Bet  4) Fold  5) All In\n");
    printf("Choice: ");
    scanf("%d", &choice);
    

    // Change request action and ammount if bet or call accordingly 
    if (choice == 1) {
        request.action = ACTION_CHECK;
        request.amount = 0;
    } else if (choice == 2) {
        request.action = ACTION_CALL;
        request.amount = 0;
    } else if (choice == 3) {
        printf("Bet amount: ");
        scanf("%d", &amount);
        if (amount < MIN_BET) amount = MIN_BET;
        request.action = ACTION_BET;
        request.amount = amount;
    } else if (choice == 4) {
        request.action = ACTION_FOLD;
        request.amount = 0;
    } else {
        request.action = ACTION_ALL_IN;
        request.amount = 0;
    }
    
    return request;

}



void getEachBet(GameState *gameState){
    ActionRequest request;

        for(int i = 0; i < gameState->playerCount; i++){
            if (!canPlayerAct(gameState, i)) continue;

            if(gameState->players[i].type == HUMAN_PLAYER){
                printGameState(gameState);
                request = getUserMove(gameState, i);
                processAction(gameState, i, request);
            }else if(gameState->players[i].type = BOT_PLAYER){
                // For now until bot logic is solidifed
                    printf("Bot %d checks.\n", i);
                    request.action = ACTION_CHECK;
                    request.amount = 0;
                    processAction(gameState, i, request);
                
            }
        }
}

void broadcastMessage(char *msg, int playerFDs[], int count)
{
    int n, l;
    l = strlen(msg);
    for (int i = 0; i < count; i++)
    {   if (playerFDs[i] >= 0)  // -1 means seat empty
        {   n = write(playerFDs[i], msg, l);
            if (n < 0)
            {   FatalError("broadcast write failed");
            }
        }
    }
}





