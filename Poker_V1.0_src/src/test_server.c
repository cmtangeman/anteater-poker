#include <stdio.h>
#include "game.h"
#include "types.h"

static int tests_run = 0;
static int tests_failed = 0;

#define ASSERT_TRUE(condition, message)                  \
    do {                                                 \
        tests_run++;                                     \
        if (!(condition)) {                              \
            tests_failed++;                              \
            printf("FAIL: %s\n", message);              \
        } else {                                         \
            printf("PASS: %s\n", message);              \
        }                                                \
    } while (0)

int main(void)
{
    GameState gameState;

    printf("Running server/game logic tests...\n");

    initializeGame(&gameState);

    ASSERT_TRUE(gameState.pot == 0, "initial pot is zero");
    ASSERT_TRUE(gameState.currentBet == 0, "initial current bet is zero");
    ASSERT_TRUE(gameState.currentTurn == 0, "initial current turn is player 0");

    initializePlayers(&gameState);

    ASSERT_TRUE(gameState.players[0].chips > 0, "player 0 receives starting chips");
    ASSERT_TRUE(gameState.players[0].folded == 0, "player 0 starts not folded");

    processBet(&gameState, 0, 10);

    ASSERT_TRUE(gameState.pot >= 10, "bet increases pot");
    ASSERT_TRUE(gameState.players[0].currentBet >= 10, "player bet is recorded");

    foldPlayer(&gameState, 0);

    ASSERT_TRUE(gameState.players[0].folded == 1, "foldPlayer marks player as folded");

    printf("Server/game tests run: %d\n", tests_run);
    printf("Server/game tests failed: %d\n", tests_failed);

    if (tests_failed == 0) {
        printf("Server test passed.\n");
        return 0;
    }

    printf("Server test failed.\n");
    return 1;
}