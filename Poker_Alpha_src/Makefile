# Makefile for Anteater Poker
# EECS 22L Team 18

CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -Isrc

SRC_DIR = src
BIN_DIR = bin

SERVER = $(BIN_DIR)/server
CLIENT = $(BIN_DIR)/poker
RULES_TEST = $(BIN_DIR)/test_rules

COMMON_SRC = \
	$(SRC_DIR)/rules.c \
	$(SRC_DIR)/deck.c \
	$(SRC_DIR)/game.c \
	$(SRC_DIR)/board.c

.PHONY: all test clean directories

all: directories $(SERVER) $(CLIENT)

directories:
	mkdir -p $(BIN_DIR)

$(SERVER): $(SRC_DIR)/poker_server.c $(COMMON_SRC) | directories
	$(CC) $(CFLAGS) $(SRC_DIR)/poker_server.c $(COMMON_SRC) -o $(SERVER)

$(CLIENT): $(SRC_DIR)/poker_client.c $(COMMON_SRC) | directories
	$(CC) $(CFLAGS) $(SRC_DIR)/poker_client.c $(COMMON_SRC) -o $(CLIENT)

$(RULES_TEST): $(SRC_DIR)/test_rules.c $(SRC_DIR)/rules.c $(SRC_DIR)/rules.h $(SRC_DIR)/types.h | directories
	$(CC) $(CFLAGS) $(SRC_DIR)/test_rules.c $(SRC_DIR)/rules.c -o $(RULES_TEST)

test: directories $(RULES_TEST)
	./$(RULES_TEST)

clean:
	rm -rf $(BIN_DIR)
	rm -f core
	rm -f core.*
