# Anteater Poker Testing

## Rules Unit Tests

The rules module is tested using src/test_rules.c.

To run the tests:

1. cd Poker_Alpha_src
2. make clean
3. make
4. make test

## Current Test Coverage

The rules unit test checks:

- get card value
- pair
- two pair
- three of a kind
- straight
- flush
- full house
- four of a kind
- straight flush
- high card negative checks
- evaluate hand for flush
- evaluate hand for full house
- evaluate hand for straight flush
- Anteater Feast special rule

## Expected Result

The expected result is:

Passed 14/14 rules tests.

## Build Verification

The Makefile builds:

- bin/server
- bin/poker
- bin/test_rules

The project can be cleaned using make clean.