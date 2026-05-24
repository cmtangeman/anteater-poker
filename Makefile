# Makefile for Anteater Poker
# EECS 22L Team 18

CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -Isrc

SRC_DIR = src
BIN_DIR = bin

RULES_TEST = $(BIN_DIR)/test_rules

.PHONY: all test clean directories

all: directories $(RULES_TEST)

directories:
	mkdir -p $(BIN_DIR)

$(RULES_TEST): $(SRC_DIR)/test_rules.c $(SRC_DIR)/rules.c $(SRC_DIR)/rules.h $(SRC_DIR)/types.h
	$(CC) $(CFLAGS) $(SRC_DIR)/test_rules.c $(SRC_DIR)/rules.c -o $(RULES_TEST)

test: all
	./$(RULES_TEST)

clean:
	rm -rf $(BIN_DIR)
	rm -f $(SRC_DIR)/test_rules
	rm -f core
	rm -f core.*
