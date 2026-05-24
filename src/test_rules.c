/*
 * test_rules.c
 *
 * Unit tests for Anteater Poker rules module.
 */

#include <stdio.h>
#include <assert.h>

#include "rules.h"
#include "types.h"

static int tests_run = 0;
static int tests_passed = 0;

#define RUN_TEST(test_function) do {                  \
    printf("Running %-35s", #test_function);          \
    tests_run++;                                      \
    test_function();                                  \
    tests_passed++;                                   \
    printf(" PASSED\n");                             \
} while (0)

static Card makeCard(Rank rank, Suit suit)
{
    Card card;
    card.rank = rank;
    card.suit = suit;
    return card;
}

void test_get_card_value(void)
{
    Card ace = makeCard(ACE, SPADES);
    Card king = makeCard(KING, HEARTS);
    Card two = makeCard(TWO, CLUBS);

    assert(getCardValue(ace) > getCardValue(king));
    assert(getCardValue(king) > getCardValue(two));
}

void test_check_pair(void)
{
    Card hand[7] = {
        makeCard(ACE, HEARTS),
        makeCard(ACE, CLUBS),
        makeCard(THREE, DIAMONDS),
        makeCard(FIVE, SPADES),
        makeCard(SEVEN, HEARTS),
        makeCard(NINE, CLUBS),
        makeCard(JACK, DIAMONDS)
    };

    assert(checkPair(hand, 7) == 1);
}

void test_check_two_pair(void)
{
    Card hand[7] = {
        makeCard(ACE, HEARTS),
        makeCard(ACE, CLUBS),
        makeCard(KING, DIAMONDS),
        makeCard(KING, SPADES),
        makeCard(SEVEN, HEARTS),
        makeCard(NINE, CLUBS),
        makeCard(JACK, DIAMONDS)
    };

    assert(checkTwoPair(hand, 7) == 1);
}

void test_check_three_of_kind(void)
{
    Card hand[7] = {
        makeCard(QUEEN, HEARTS),
        makeCard(QUEEN, CLUBS),
        makeCard(QUEEN, DIAMONDS),
        makeCard(FIVE, SPADES),
        makeCard(SEVEN, HEARTS),
        makeCard(NINE, CLUBS),
        makeCard(JACK, DIAMONDS)
    };

    assert(checkThreeOfKind(hand, 7) == 1);
}

void test_check_straight(void)
{
    Card hand[7] = {
        makeCard(FIVE, HEARTS),
        makeCard(SIX, CLUBS),
        makeCard(SEVEN, DIAMONDS),
        makeCard(EIGHT, SPADES),
        makeCard(NINE, HEARTS),
        makeCard(KING, CLUBS),
        makeCard(ACE, DIAMONDS)
    };

    assert(checkStraight(hand, 7) == 1);
}

void test_check_flush(void)
{
    Card hand[7] = {
        makeCard(TWO, HEARTS),
        makeCard(FIVE, HEARTS),
        makeCard(SEVEN, HEARTS),
        makeCard(NINE, HEARTS),
        makeCard(KING, HEARTS),
        makeCard(THREE, CLUBS),
        makeCard(ACE, DIAMONDS)
    };

    assert(checkFlush(hand, 7) == 1);
}

void test_check_full_house(void)
{
    Card hand[7] = {
        makeCard(TEN, HEARTS),
        makeCard(TEN, CLUBS),
        makeCard(TEN, DIAMONDS),
        makeCard(KING, SPADES),
        makeCard(KING, HEARTS),
        makeCard(THREE, CLUBS),
        makeCard(ACE, DIAMONDS)
    };

    assert(checkFullHouse(hand, 7) == 1);
}

void test_check_four_of_kind(void)
{
    Card hand[7] = {
        makeCard(NINE, HEARTS),
        makeCard(NINE, CLUBS),
        makeCard(NINE, DIAMONDS),
        makeCard(NINE, SPADES),
        makeCard(KING, HEARTS),
        makeCard(THREE, CLUBS),
        makeCard(ACE, DIAMONDS)
    };

    assert(checkFourOfKind(hand, 7) == 1);
}

void test_check_straight_flush(void)
{
    Card hand[7] = {
        makeCard(FIVE, SPADES),
        makeCard(SIX, SPADES),
        makeCard(SEVEN, SPADES),
        makeCard(EIGHT, SPADES),
        makeCard(NINE, SPADES),
        makeCard(KING, HEARTS),
        makeCard(ACE, DIAMONDS)
    };

    assert(checkStraightFlush(hand, 7) == 1);
}

void test_high_card_negative_checks(void)
{
    Card hand[7] = {
        makeCard(TWO, HEARTS),
        makeCard(FOUR, CLUBS),
        makeCard(SIX, DIAMONDS),
        makeCard(EIGHT, SPADES),
        makeCard(TEN, HEARTS),
        makeCard(QUEEN, CLUBS),
        makeCard(ACE, DIAMONDS)
    };

    assert(checkPair(hand, 7) == 0);
    assert(checkTwoPair(hand, 7) == 0);
    assert(checkThreeOfKind(hand, 7) == 0);
    assert(checkStraight(hand, 7) == 0);
    assert(checkFlush(hand, 7) == 0);
    assert(checkFullHouse(hand, 7) == 0);
    assert(checkFourOfKind(hand, 7) == 0);
    assert(checkStraightFlush(hand, 7) == 0);
}

void test_evaluate_hand_flush(void)
{
    Card hand[7] = {
        makeCard(TWO, HEARTS),
        makeCard(FIVE, HEARTS),
        makeCard(SEVEN, HEARTS),
        makeCard(NINE, HEARTS),
        makeCard(KING, HEARTS),
        makeCard(THREE, CLUBS),
        makeCard(ACE, DIAMONDS)
    };

    assert(evaluateHand(hand, 7) == FLUSH);
}

void test_evaluate_hand_full_house(void)
{
    Card hand[7] = {
        makeCard(TEN, HEARTS),
        makeCard(TEN, CLUBS),
        makeCard(TEN, DIAMONDS),
        makeCard(KING, SPADES),
        makeCard(KING, HEARTS),
        makeCard(THREE, CLUBS),
        makeCard(ACE, DIAMONDS)
    };

    assert(evaluateHand(hand, 7) == FULL_HOUSE);
}

void test_evaluate_hand_straight_flush(void)
{
    Card hand[7] = {
        makeCard(FIVE, SPADES),
        makeCard(SIX, SPADES),
        makeCard(SEVEN, SPADES),
        makeCard(EIGHT, SPADES),
        makeCard(NINE, SPADES),
        makeCard(KING, HEARTS),
        makeCard(ACE, DIAMONDS)
    };

    assert(evaluateHand(hand, 7) == STRAIGHT_FLUSH);
}

void test_check_anteater_feast(void)
{
    Card hand[7] = {
        makeCard(ANT, HEARTS),
        makeCard(ANT, DIAMONDS),
        makeCard(ANT, CLUBS),
        makeCard(ANT, SPADES),
        makeCard(Anteater, HEARTS),
        makeCard(KING, CLUBS),
        makeCard(QUEEN, DIAMONDS)
    };

    assert(checkAnteaterFeast(hand, 7) == 1);
}
int main(void)
{
    printf("========================================\n");
    printf(" Anteater Poker Rules Unit Tests\n");
    printf("========================================\n");

    RUN_TEST(test_get_card_value);
    RUN_TEST(test_check_pair);
    RUN_TEST(test_check_two_pair);
    RUN_TEST(test_check_three_of_kind);
    RUN_TEST(test_check_straight);
    RUN_TEST(test_check_flush);
    RUN_TEST(test_check_full_house);
    RUN_TEST(test_check_four_of_kind);
    RUN_TEST(test_check_straight_flush);
    RUN_TEST(test_high_card_negative_checks);
    RUN_TEST(test_evaluate_hand_flush);
    RUN_TEST(test_evaluate_hand_full_house);
    RUN_TEST(test_evaluate_hand_straight_flush);
    RUN_TEST(test_check_anteater_feast);

    printf("========================================\n");
    printf("Passed %d/%d rules tests.\n", tests_passed, tests_run);
    printf("========================================\n");

    return 0;
}
