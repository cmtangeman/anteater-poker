/* Gutted and reworked version of ClockServer.c: simple TCP/IP server example with timeout support
 * Author: Rainer Doemer, 5/15/23 (prior versions 2/17/15, 2/20/17)
 Charlie Ta
 gcc bot_helper.c bot.c deck.c game.c rules.c poker_server.c -o poker_server_cli
 */

// Todo, ensure program works with just one client and also only plays one game at a time

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <netinet/in.h>
#include <netdb.h>
#include <time.h>
#include <sys/select.h>
#include <arpa/inet.h>
#include <assert.h>

#include "poker_server.h"
#include "types.h"
#include "game.h"
#include "bot.h"

int playerFDS[FD_SETSIZE];
int playerCount = 0;
int gameInProgress = 0;
int actionsThisRound;
GameState gameState;

int playersThisPhase;

int runBotActions(GameState *gs);

// Char arrays to match up strings with enums
const char *rankNames[] = {"Ant", "2", "3", "4", "5", "6", "7", "8", 
                            "9", "10", "J", "Q", "K", "A", "Anteater"};
const char *suitNames[] = {"Hearts", "Diamonds", "Clubs", "Spades"};

// Different client states that dictate what message tree to go down 
typedef enum {
    STATE_CONNECTED,   
    STATE_LOBBY,      
    STATE_PLAYING,
    STATE_GAME  
} ClientState;

ClientState clientStates[FD_SETSIZE];   // Check state of each client depending on their FD


/* #define DEBUG */	/* be verbose */

/*** global variables ****************************************************/

const char *Program	/* program name for descriptive diagnostics */
	= NULL;
int Shutdown		/* keep running until Shutdown == 1 */
	= 0;
char ClockBuffer[26]	/* current time in printable format */
	= "";

/*** global functions ****************************************************/

// Use when string is not pre loaded
void sendMsg(int fd, const char *msg)
{   write(fd, msg, strlen(msg));
}

// This will send each turn and give the player all the info they need in one message
// TODO: Instead of sending a message like this, update the gui terminal
// Also update the button for bet for example to turn green, from red when
// it wasnt the users turn

void sendPlayerStatus(GameState *gs, int playerIndex)
{
    char SendBuf[1024] = "";
    char temp[128];
    const char *phaseNames[] = {"Waiting", "Preflop", "Flop", "Turn", "River", "Showdown", "Over"};

    // round and community cards 
    snprintf(temp, sizeof(temp), "--- %s ---\n", phaseNames[gs->phase]);
    strncat(SendBuf, temp, sizeof(SendBuf) - strlen(SendBuf) - 1);

    if (gs->communityCardCount > 0)
    {   strncat(SendBuf, "Community: ", sizeof(SendBuf) - strlen(SendBuf) - 1);
        for (int i = 0; i < gs->communityCardCount; i++)
        {   snprintf(temp, sizeof(temp), "%s of %s  ",
                    rankNames[gs->communityCards[i].rank],
                    suitNames[gs->communityCards[i].suit]);
            strncat(SendBuf, temp, sizeof(SendBuf) - strlen(SendBuf) - 1);
        }
        strncat(SendBuf, "\n", sizeof(SendBuf) - strlen(SendBuf) - 1);
    }

    //player status 
    snprintf(temp, sizeof(temp),
        "%s's STATUS:\n"
        "Hand: %s of %s, %s of %s\n"
        "Chips: $%d\n"
        "Pot: $%d | Current Bet: $%d\n"
        "1)Check 2)Call 3)Bet 4)Fold 5)AllIn\n",
        gs->players[playerIndex].username,
        rankNames[gs->players[playerIndex].hand[0].rank],
        suitNames[gs->players[playerIndex].hand[0].suit],
        rankNames[gs->players[playerIndex].hand[1].rank],
        suitNames[gs->players[playerIndex].hand[1].suit],
        gs->players[playerIndex].chips,
        gs->pot,
        gs->currentBet);
    strncat(SendBuf, temp, sizeof(SendBuf) - strlen(SendBuf) - 1);

    write(playerFDS[playerIndex], SendBuf, strlen(SendBuf));
}

int recvMsg(int fd, char *buf, int size)
{   int n = read(fd, buf, size-1);
    if (n > 0) buf[n] = 0;
    return n;
}

void broadcastMessage(char *msg, int playerFDs[], int count)
{
    for (int i = 0; i < count; i++)
    {   if (playerFDs[i] >= 0)
        {   write(playerFDs[i], msg, strlen(msg));
        }
    }
}


int findSeat(int fd)
{   for (int i = 0; i < playerCount; i++)
    {   if (playerFDS[i] == fd) return i;
    }
    return -1;
}

void startPokerGame(void);


void FatalError(		/* print error diagnostics and abort */
	const char *ErrorMsg)
{
    fputs(Program, stderr);
    fputs(": ", stderr);
    perror(ErrorMsg);
    fputs(Program, stderr);
    fputs(": Exiting!\n", stderr);
    exit(20);
} /* end of FatalError */

int MakeServerSocket(		/* create a socket on this server */
        
	uint16_t PortNo)    // This will run only for the server side
{
    int ServSocketFD;
    struct sockaddr_in ServSocketName;

    /* create the socket */
    ServSocketFD = socket(PF_INET, SOCK_STREAM, 0); // domain, type, protocol family: IpV4 --> , TCP socket type, protocol: let OS pick default 
    if (ServSocketFD < 0)
    {   FatalError("service socket creation failed");
    }
    /* bind the socket to this server */
    // Adress family same as protocol family 
    ServSocketName.sin_family = AF_INET;
    ServSocketName.sin_port = htons(PortNo);
    ServSocketName.sin_addr.s_addr = htonl(INADDR_ANY);
    if (bind(ServSocketFD, (struct sockaddr*)&ServSocketName,   //Attach socket to specific IP addr and port locally 
		sizeof(ServSocketName)) < 0)
    {   FatalError("binding the server to a socket failed");
    }
    /* start listening to this socket */
    if (listen(ServSocketFD, 5) < 0)	/* max 5 clients in backlog */
    {   FatalError("listening on socket failed");
    }
    return ServSocketFD;
} /* end of MakeServerSocket */

int ProcessRequest(		/* process an input request by a client and return once done */
	int DataSocketFD)
{
    int  l, n, seat, startGame;
    char RecvBuf[256];	/* message buffer for receiving a message */
    char SendBuf[256];	/* message buffer for sending a response */

    ActionRequest request;

    printf("State: %d FD: %d\n", clientStates[DataSocketFD], DataSocketFD);

    n = read(DataSocketFD, RecvBuf, sizeof(RecvBuf)-1); // read returns n>0 bytes sucessfuly read, n <- a system occured
    if (n < 0)  // n == 0 connection close by the other end ( EOF)
    {   FatalError("reading from data socket failed");
    return 0;
    }
    RecvBuf[n] = 0;

    if (0 == strcmp(RecvBuf, "SHUTDOWN"))
{   Shutdown = 1;
    sendMsg(DataSocketFD, "OK SHUTDOWN");
    return 0;
}

    // debug
    printf("%s: Received message: %s\n", Program, RecvBuf);



    
    switch(clientStates[DataSocketFD])
    {

        case STATE_CONNECTED: 

        if (gameInProgress)
        {   sendMsg(DataSocketFD, "Game in progress, try later\n");
            return 0;
        }

            if (0 == strcmp(RecvBuf, "START"))
        {   strncpy(SendBuf, "ENTER YOUR NAME: ", sizeof(SendBuf)-1);

            SendBuf[sizeof(SendBuf)-1] = 0;
            l = strlen(SendBuf);
            n = write(DataSocketFD, SendBuf, l);
            clientStates[DataSocketFD] = STATE_LOBBY;
            return 1;  /* keep open, wait for next message */
        }

        else
        { // handle invalid input from user
            sendMsg(DataSocketFD, "Invalid input. Please type START, or choose from the options available.\n");
            clientStates[DataSocketFD] = STATE_CONNECTED;
            return 1;
    
        }

        break;


        
        case STATE_LOBBY:

            // Get the unique ID
            strncpy(gameState.players[playerCount].username, RecvBuf,
            sizeof(gameState.players[0].username) - 1);
            // Initialize seat
 
            seat = playerCount;
            gameState.players[seat].seat = playerCount;
            playerCount++;
            playerFDS[seat] = DataSocketFD;

            printf("New player: %s at seat %d\n", gameState.players[seat].username, seat);
            snprintf(SendBuf, sizeof(SendBuf), "Welcome %s Input READY to begin game", RecvBuf);
            n = write(DataSocketFD, SendBuf, strlen(SendBuf));
            if (n < 0)
            {   FatalError("writing to data socket failed");
            }
            clientStates[DataSocketFD] = STATE_PLAYING;
            return 1;
            
            // sendMsg(DataSocketFD, SendBuf);
            // break back to main in which we will return 1 thereby keep listening to the server. 
            // We will wait for 
            break;


        case STATE_PLAYING:
            
            seat = findSeat(DataSocketFD);
            
            if (0 == strcmp(RecvBuf, "READY"))
            {
            
            startGame = 1; 
            gameState.players[seat].ready = 1;
            
            for(int i = 0; i < playerCount; i++){
                if(gameState.players[i].ready != 1){
                    startGame = 0;
                    break;
            }
            }

            if(startGame)
            {

            // Initialize Poker 
            startPokerGame();
            printf("Seat: %i playerFDS[seat] : %i ", seat, playerFDS[seat]);
            startRound(&gameState);
            playersThisPhase = countActivePlayers(&gameState);
            actionsThisRound = 0;
            clientStates[DataSocketFD] = STATE_GAME;
            for (int j = 0; j < playerCount; j++)   // Set players who are waiting to be in game_State.
            clientStates[playerFDS[j]] = STATE_GAME;
            gameInProgress = 1;
            printf("%i", gameState.currentTurn);

            broadcastMessage("Game starting!\n", playerFDS, playerCount);
            sendPlayerStatus(&gameState, gameState.currentTurn);



            }else
            {
            // sendMsg(DataSocketFD, "Waiting for other players to ready up...\n");
            }




            }
            else
            {   
            sendMsg(DataSocketFD, "Input READY to begin game\n");
            }

            break;

    case STATE_GAME:
            
            
            {
            
            
            // sendPlayerStatus(&gameState, findSeat(DataSocketFD));  -
            // recvMsg(playerFDS[DataSocketFD], RecvBuf, sizeof(RecvBuf));  // then waits for action
            // Can already read as we have processed above. 
            int choice = atoi(RecvBuf);
            
            seat = findSeat(DataSocketFD);

            if (seat != gameState.currentTurn) {
                sendMsg(DataSocketFD, "Not your turn.\n");
                return 1;
            }



            if (choice == 1)      { request.action = ACTION_CHECK;  request.amount = 0; }
            else if (choice == 2) { request.action = ACTION_CALL;   request.amount = 0; }
            else if (choice == 3)
            {   
                // Receive a 2nd Message
                recvMsg(DataSocketFD, RecvBuf, sizeof(RecvBuf));
                request.action = ACTION_BET;
                request.amount = atoi(RecvBuf);
                // if (request.amount < MIN_BET) request.amount = MIN_BET;
            }
            else if (choice == 4) { request.action = ACTION_FOLD;   request.amount = 0; }
            else                  { request.action = ACTION_ALL_IN; request.amount = 0; }

            processAction(&gameState, seat, request);   // update the board accordingly 
            actionsThisRound++;

            // check for 1 remaining player
            if (countActivePlayers(&gameState) == 1) {

                int winner = -1; // didnt find winner still
            
                // check each player, for who is the only active remaining
                for (int i = 0; i < gameState.playerCount; i++) {
                    //who didnt fold
                    if (!gameState.players[i].folded) {
                        winner = i;
                        break;
                    }
                }

                // pot to winner
                gameState.players[winner].chips += gameState.pot;

                // create string for user and print out winner message
                char buff[256];
                snprintf(buff, sizeof(buff), "%s is the winner. Everyone else folded!\n", gameState.players[winner].username);
                broadcastMessage(buff, playerFDS, playerCount);
                gameInProgress = 0;

                endRound(&gameState, winner);
                return 1;
            }

            actionsThisRound += runBotActions(&gameState);

                        // check for 1 remaining player
            if (countActivePlayers(&gameState) == 1) {

                int winner = -1; // didnt find winner still
            
                // check each player, for who is the only active remaining
                for (int i = 0; i < gameState.playerCount; i++) {
                    //who didnt fold
                    if (!gameState.players[i].folded) {
                        winner = i;
                        break;
                    }
                }

                // pot to winner
                gameState.players[winner].chips += gameState.pot;

                // create string for user and print out winner message
                char buff[256];
                snprintf(buff, sizeof(buff), "%s is the winner. Everyone else folded!\n", gameState.players[winner].username);
                broadcastMessage(buff, playerFDS, playerCount);
                endRound(&gameState, winner);
                
                return 1;
            }
            // TODO: Make a helper function to check for a winner, instead of repeating twice.

            printf("Actions this round: %d / %d\n", actionsThisRound, playersThisPhase);   
            // runBotActions(&gameState);
            if (actionsThisRound >= playersThisPhase)       // To prevent folds from dynamically decreasing playersThisPhase
            {   
                
                actionsThisRound = 0;
                playersThisPhase = countActivePlayers(&gameState);
                advancePhase(&gameState);
                printf("Phase advanced to %d\n", gameState.phase);
                // startRound(&gameState);

            if (gameState.phase == GAME_SHOWDOWN)
            {
                int winner = determineWinner(&gameState);
                char buf[256];
                snprintf(buf, sizeof(buf), "Winner: %s!\n", gameState.players[winner].username);
                broadcastMessage(buf, playerFDS, playerCount);
                endRound(&gameState, winner);
                gameInProgress = 0;  // allow new game
                for (int j = 0; j < playerCount; j++)
                clientStates[playerFDS[j]] = STATE_CONNECTED;
                broadcastMessage("Type START to play again.\n", playerFDS, playerCount);
                return 1;  // keep sockets open
            }
                            
            }
        

            if (gameState.players[seat].type == HUMAN_PLAYER){
                sendPlayerStatus(&gameState, gameState.currentTurn);
            }

            
            break;


            }

    }

    return 1;

/*
#ifdef DEBUG
    printf("%s: Sending response: %s.\n", Program, SendBuf);
#endif
*/

} /* end of ProcessRequest */

void ServerMainLoop(		/* simple server main loop */
	int ServSocketFD,		/* server socket to wait on */
	int Timeout)			/* timeout in micro seconds */
{
    int DataSocketFD;	/* socket for a new client */
    socklen_t ClientLen;
    struct sockaddr_in
	ClientAddress;	/* client address we connect with */
    fd_set ActiveFDs;	/* socket file descriptors to select from (Tracks multiple open socket FD's simultaneously) */ 
    fd_set ReadFDs;	/* socket file descriptors ready to read from */
    struct timeval TimeVal;
    int res, i;

    FD_ZERO(&ActiveFDs);		/* Zero out set of active sockets-> initialize */
    FD_SET(ServSocketFD, &ActiveFDs);	/* server socket is active */
                                        // Adds ServSocketFD to ActiveFDs bitmask
                                        // Now ActiveFDs has only one FD marked active
                                        // 
    while(!Shutdown)
    {   ReadFDs = ActiveFDs;
	TimeVal.tv_sec  = Timeout / 1000000;	/* seconds */
	TimeVal.tv_usec = Timeout % 1000000;	/* microseconds */
	/* block until input arrives on active sockets or until timeout */
    // The 250ms is the maximum (Timeout) it'll wait before 
    // giving up and returning res == 0 to update the clock.
    // Read FDS
	res = select(FD_SETSIZE, &ReadFDs, NULL, NULL, &TimeVal);   // Number of FDs that are ready to read  
    //
	if (res < 0)
	{   FatalError("wait for input or timeout (select) failed");
	}
	if (res == 0)	/* timeout occurred */
	{
        // printf("No Message for 250ms");
        // Do nothing 
        
	}
	else		/* some FDs have data ready to read res (1+)*/
	{   for(i=0; i<FD_SETSIZE; i++) // (5 in our case)
	    {   if (FD_ISSET(i, &ReadFDs))  //Checs if i is in ReadFDS (One of the FDs that are ready)
		{   if (i == ServSocketFD)
		    {	/* connection request on server socket */
#ifdef DEBUG    // Gate DEBUG STATEMENTS
                // Only fires when the binary is compiled with --DEBUG 
			printf("%s: Accepting new client %d...\n", Program, i);
#endif
			ClientLen = sizeof(ClientAddress);
			DataSocketFD = accept(ServSocketFD, // Client is now active on 
				(struct sockaddr*)&ClientAddress, &ClientLen);
			if (DataSocketFD < 0)
			{   FatalError("data socket creation (accept) failed");
			}
// DEBUG
			printf("%s: Client %d connected from %s:%hu.\n",
				Program, i,
				inet_ntoa(ClientAddress.sin_addr),
				ntohs(ClientAddress.sin_port));

			FD_SET(DataSocketFD, &ActiveFDs);
		    }
		    else
		    {   /* active communication with a client */
            int result = ProcessRequest(i);
            if (result == 0)
            {   close(i);
                FD_CLR(i, &ActiveFDs);  // Else we keep running main, server loop
            }
            }
		}
	    }
	}
    }
} /* end of ServerMainLoop */

/*** main function *******************************************************/

int main(int argc, char *argv[])
{
    int ServSocketFD;	/* socket file descriptor for service */
    int PortNo;		/* port number */

    Program = argv[0];	/* publish program name (for diagnostics) */
#ifdef DEBUG
    printf("%s: Starting...\n", Program);
#endif
    if (argc < 2)
    {   fprintf(stderr, "Usage: %s port\n", Program);
	exit(10);
    }
    PortNo = atoi(argv[1]);	/* get the port number */
    if (PortNo <= 2000)
    {   fprintf(stderr, "%s: invalid port number %d, should be >2000\n",
		Program, PortNo);
        exit(10);
    }
#ifdef DEBUG
    printf("%s: Creating the server socket...\n", Program);
#endif
    ServSocketFD = MakeServerSocket(PortNo);
    // Initialize players as not there yet
    printf("%s: Running anteater poker at port %d...\n", Program, PortNo);
    for (int i = 0; i < MAX_PLAYERS; i++){
    playerFDS[i] = -1;
    } 
    
    // Goes into this loop until server is shutdown
    ServerMainLoop(ServSocketFD, 250000);
    printf("\n%s: Shutting down.\n", Program);
    close(ServSocketFD);
    return 0;
}

/* EOF poker_server.c */





// Networking Poker Logic: 


void startPokerGame(void)
{
    char SendBuf[256];

    initializeGame(&gameState);

    // Initialize the players with some money 
    for(int j = 0; j < playerCount; j++){
        gameState.players[j].chips = 20;
        printf(" %s's chips initialized at %i \n", gameState.players[j].username, gameState.players[j].chips);
    }

    // fill remaining seats with bots 
    for (int i = playerCount; i < MAX_PLAYERS; i++)
    {   gameState.players[i].type = BOT_PLAYER;
        snprintf(gameState.players[i].username, 
                sizeof(gameState.players[i].username), "Bot %d", i);
        gameState.players[i].seat = i;
        printf("Bot %d initialized at %d \n", i, gameState.players[i].seat);
        gameState.players[i].chips = 20;
        printf("Bot %d initialized with %d chips \n", i, gameState.players[i].chips);
    }

    gameState.playerCount = MAX_PLAYERS;
    

}

int runBotActions(GameState *gs)
{
    int botsActed = 0;
    ActionRequest request;
    Card allCards[7];
    int cardCount;

    int safety = 0; //prevent loop
    // keep processing until it's a human's turn or round ends 
    while (gs->players[gs->currentTurn].type == BOT_PLAYER
           && !gs->players[gs->currentTurn].folded //only when not folded do something
           && countActivePlayers(gs) > 1
           && gs->phase != GAME_OVER)
    {
        safety++;
        
        cardCount = 0;
        allCards[cardCount++] = gs->players[gs->currentTurn].hand[0];
        allCards[cardCount++] = gs->players[gs->currentTurn].hand[1];

        for (int j = 0; j < gs->communityCardCount; j++)
            allCards[cardCount++] = gs->communityCards[j];

        request = easyMode(&gs->players[gs->currentTurn], allCards, cardCount);
        processAction(gs, gs->currentTurn, request);
        botsActed++;
    }

    return botsActed;
}