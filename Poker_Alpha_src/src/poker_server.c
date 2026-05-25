/* ClockServer.c: simple TCP/IP server example with timeout support
 * Author: Rainer Doemer, 5/15/23 (prior versions 2/17/15, 2/20/17)
 */

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

#include "types.h"
#include "game.h"
#include "deck.h"



/* #define DEBUG */	/* be verbose */

/*** global variables ****************************************************/

const char *Program	/* program name for descriptive diagnostics */
	= NULL;
int Shutdown		/* keep running until Shutdown == 1 */
	= 0;

// Game Globals:
GameState gameState;
int clientSockets[MAX_PLAYERS];
int numConnected = 0;

char *suitName(int s) {
    if (s == 0) return "Hearts";
    if (s == 1) return "Diamonds";
    if (s == 2) return "Clubs";
    return "Spades";
}

char *rankName(int r) {
    char *ranks[] = {"2","3","4","5","6","7","8","9","10",
                     "J","Q","K","A","Anteater"};
    if (r >= 0 && r < 14) return ranks[r];
    return "?";
}
/*** global functions ****************************************************/

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
	uint16_t PortNo)
{
    int ServSocketFD;
    struct sockaddr_in ServSocketName;

    /* create the socket */
    ServSocketFD = socket(PF_INET, SOCK_STREAM, 0);
    if (ServSocketFD < 0)
    {   FatalError("service socket creation failed");
    }
    /* bind the socket to this server */
    ServSocketName.sin_family = AF_INET;
    ServSocketName.sin_port = htons(PortNo);
    ServSocketName.sin_addr.s_addr = htonl(INADDR_ANY);
    if (bind(ServSocketFD, (struct sockaddr*)&ServSocketName,
		sizeof(ServSocketName)) < 0)
    {   FatalError("binding the server to a socket failed");
    }
    /* start listening to this socket */
    if (listen(ServSocketFD, 5) < 0)	/* max 5 clients in backlog */
    {   FatalError("listening on socket failed");
    }
    return ServSocketFD;
} /* end of MakeServerSocket  */


void SendGameState(int fd) {
    char buf[2048];
    char line[256];
    int i;

    snprintf(buf, sizeof(buf), "\n--- Community Cards ---\n");
    for (i = 0; i < gameState.communityCardCount; i++) {
        snprintf(line, sizeof(line), "%s of %s\n",
                 rankName(gameState.communityCards[i].rank),
                 suitName(gameState.communityCards[i].suit));
        strncat(buf, line, sizeof(buf)-strlen(buf)-1);
    }
    snprintf(line, sizeof(line), "Pot: %d chips\nCurrent Bet: %d chips\n",
             gameState.pot, gameState.currentBet);
    strncat(buf, line, sizeof(buf)-strlen(buf)-1);

    strncat(buf, "\n--- Players ---\n", sizeof(buf)-strlen(buf)-1);
    for (i = 0; i < gameState.playerCount; i++) {
        snprintf(line, sizeof(line), "Player %d: %s - Chips: %d%s%s\n",
                 i,
                 gameState.players[i].username,
                 gameState.players[i].chips,
                 gameState.players[i].folded ? " [FOLDED]" : "",
                 gameState.players[i].allIn  ? " [ALL IN]" : "");
        strncat(buf, line, sizeof(buf)-strlen(buf)-1);
    }

    write(fd, buf, strlen(buf));
}

void ProcessRequest(int fd){		/* process a time request by a client */
	int DataSocketFD = fd;
    int  l, n;
    char RecvBuf[256];	/* message buffer for receiving a message */
    char SendBuf[256];	/* message buffer for sending a response */

    n = read(DataSocketFD, RecvBuf, sizeof(RecvBuf)-1);
	
    if (n < 0) 
    {   FatalError("reading from data socket failed");
    }

    RecvBuf[n] = 0;
	
#ifdef DEBUG
    printf("%s: Received message: %s\n", Program, RecvBuf);
#endif
    if (0 == strcmp(RecvBuf, "SHUTDOWN"))
    {   Shutdown = 1;
	strncpy(SendBuf, "OK SHUTDOWN", sizeof(SendBuf)-1);
	SendBuf[sizeof(SendBuf)-1] = 0;
    }
	else if (strcmp(RecvBuf, "READY") == 0) {
        /* ask for username */
        write(fd, "USERNAME\n", 9);

    } else if (strncmp(RecvBuf, "NAME ", 5) == 0) {
        /* find this player and set username */
        for (int i = 0; i < numConnected; i++) {
            if (clientSockets[i] == fd) {
                strncpy(gameState.players[i].username,
                        RecvBuf + 5, MAX_USERNAME_LEN - 1);
                printf("%s: Player %d is '%s'\n", Program, i,
                       gameState.players[i].username);
                snprintf(SendBuf, sizeof(SendBuf),
                         "Welcome %s!\n", gameState.players[i].username);
                write(fd, SendBuf, strlen(SendBuf)); // IMPORTANT
                SendGameState(fd);
                break;
            }
        }
	}
    else
    {   strncpy(SendBuf, "ERROR unknown command ", sizeof(SendBuf)-1);
	SendBuf[sizeof(SendBuf)-1] = 0;
	strncat(SendBuf, RecvBuf, sizeof(SendBuf)-1-strlen(SendBuf));
    }
}
/*
    l = strlen(SendBuf);
#ifdef DEBUG
    printf("%s: Sending response: %s.\n", Program, SendBuf);
#endif
    if (n < 0)
    {   FatalError("writing to data socket failed");
    }
}
 /* end of ProcessRequest */

void ServerMainLoop(		/* simple server main loop */
	int ServSocketFD,		/* server socket to wait on */
	int Timeout)			/* timeout in micro seconds */
{
    int DataSocketFD;	/* socket for a new client */
    socklen_t ClientLen;
    struct sockaddr_in
	ClientAddress;	/* client address we connect with */
    fd_set ActiveFDs;	/* socket file descriptors to select from */
    fd_set ReadFDs;	/* socket file descriptors ready to read from */
    struct timeval TimeVal;
    int res, i;

    FD_ZERO(&ActiveFDs);		/* set of active sockets */
    FD_SET(ServSocketFD, &ActiveFDs);	/* server socket is active */
	write(DataSocketFD, "CONNECTED\nSend READY to join.\n", 30);
	if (numConnected < MAX_PLAYERS)
    clientSockets[numConnected++] = DataSocketFD;
    while(!Shutdown)
    {   ReadFDs = ActiveFDs;
	TimeVal.tv_sec  = Timeout / 1000000;	/* seconds */
	TimeVal.tv_usec = Timeout % 1000000;	/* microseconds */
	/* block until input arrives on active sockets or until timeout */
	res = select(FD_SETSIZE, &ReadFDs, NULL, NULL, &TimeVal);
	if (res < 0)
	{   FatalError("wait for input or timeout (select) failed");
	}
	if (res == 0)	/* timeout occurred */
	{
#ifdef DEBUG
	    printf("%s: Handling timeout...\n", Program);
#endif
	    // PrintCurrentTime();
		printf("%s: Waiting...\n", Program);
	}
	else		/* some FDs have data ready to read */
	{   for(i=0; i<FD_SETSIZE; i++)
	    {   if (FD_ISSET(i, &ReadFDs))
		{   if (i == ServSocketFD)
		    {	/* connection request on server socket */
#ifdef DEBUG
			printf("%s: Accepting new client %d...\n", Program, i);
#endif
			ClientLen = sizeof(ClientAddress);
			DataSocketFD = accept(ServSocketFD,
				(struct sockaddr*)&ClientAddress, &ClientLen);
			if (DataSocketFD < 0)
			{   FatalError("data socket creation (accept) failed");
			}
#ifdef DEBUG
			printf("%s: Client %d connected from %s:%hu.\n",
				Program, i,
				inet_ntoa(ClientAddress.sin_addr),
				ntohs(ClientAddress.sin_port));
#endif
			FD_SET(DataSocketFD, &ActiveFDs);
		    }
		    else
		    {   /* active communication with a client */
#ifdef DEBUG
			printf("%s: Dealing with client %d...\n", Program, i);
#endif
			ProcessRequest(i);
#ifdef DEBUG
			printf("%s: Closing client %d connection.\n", Program, i);
#endif
			// close(i); Keep Open for game 
			// FD_CLR(i, &ActiveFDs);
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
    printf("%s: Providing current time at port %d...\n", Program, PortNo);
	initializeGame(&gameState);
    ServerMainLoop(ServSocketFD, 250000);
    printf("\n%s: Shutting down.\n", Program);
    close(ServSocketFD);
    return 0;
}

/* EOF ClockServer.c */