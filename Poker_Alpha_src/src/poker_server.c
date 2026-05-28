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

#include "poker_server.h"
#include "types.h"
#include "game.h"

int playerFDS[FD_SETSIZE];
int playerCount = 0;
GameState gameState;

typedef enum {
    STATE_CONNECTED,   
    STATE_LOBBY,      
    STATE_PLAYING      
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
} /* end of MakeServerSocket */

void PrintCurrentTime(void)	/*  print/update the current real time */
{
    time_t CurrentTime; /* seconds since 1970 (see 'man 2 time') */
    char   *TimeString;	/* printable time string (see 'man ctime') */
    char   Wheel,
	   *WheelChars = "|/-\\";
    static int WheelIndex = 0;

    CurrentTime = time(NULL);	/* get current real time (in seconds) */
    TimeString = ctime(&CurrentTime);	/* convert to printable format */
    strncpy(ClockBuffer, TimeString, 25);
    ClockBuffer[24] = 0;	/* remove unwanted '/n' at the end */
    WheelIndex = (WheelIndex+1) % 4;
    Wheel = WheelChars[WheelIndex];
    printf("\rClock: %s %c",	/* print from beginning of current line */
	ClockBuffer, Wheel);	/* print time plus a rotating wheel */
    fflush(stdout);
} /* end of PrintCurrentTime */

int ProcessRequest(		/* process an input request by a client and return once done */
	int DataSocketFD)
{
    int  l, n;
    char RecvBuf[256];	/* message buffer for receiving a message */
    char SendBuf[256];	/* message buffer for sending a response */

    n = read(DataSocketFD, RecvBuf, sizeof(RecvBuf)-1); // read returns n>0 bytes sucessfuly read, n <- a system occured
    if (n < 0)  // n == 0 connection close by the other end ( EOF)
    {   FatalError("reading from data socket failed");
    return 0;
    }
    RecvBuf[n] = 0;
#ifdef DEBUG
    printf("%s: Received message: %s\n", Program, RecvBuf);
#endif
    if (0 == strcmp(RecvBuf, "TIME"))
    {   strncpy(SendBuf, "OK TIME: ", sizeof(SendBuf)-1);
	SendBuf[sizeof(SendBuf)-1] = 0;
	strncat(SendBuf, ClockBuffer, sizeof(SendBuf)-1-strlen(SendBuf));
    l = strlen(SendBuf);
    n = write(DataSocketFD, SendBuf, l);
    if (n < 0)
    {   FatalError("writing to data socket failed");
    }
    return 0;
    }
    else if (0 == strcmp(RecvBuf, "SHUTDOWN"))
    {   Shutdown = 1;
	strncpy(SendBuf, "OK SHUTDOWN", sizeof(SendBuf)-1);
	SendBuf[sizeof(SendBuf)-1] = 0;
    l = strlen(SendBuf);
    n = write(DataSocketFD, SendBuf, l);
    if (n < 0)
    {   FatalError("writing to data socket failed");
    }
    return 0;
    }
    /*
    else
    {   strncpy(SendBuf, "ERROR unknown command ", sizeof(SendBuf)-1);
	SendBuf[sizeof(SendBuf)-1] = 0;
	strncat(SendBuf, RecvBuf, sizeof(SendBuf)-1-strlen(SendBuf));
   
    l = strlen(SendBuf);
    n = write(DataSocketFD, SendBuf, l);
    if (n < 0)
    {   FatalError("writing to data socket failed");
    }
    return 0;
    }
    */
    
    switch(clientStates[DataSocketFD])
    {
        case STATE_CONNECTED:
            // handle username input
            // Joining lobby
            strncpy(SendBuf, "ENTER YOUR NAME:", sizeof(SendBuf)-1);
            SendBuf[sizeof(SendBuf)-1] = 0;
            l = strlen(SendBuf);
            n = write(DataSocketFD, SendBuf, l);
            if (n < 0)
            {   FatalError("writing to data socket failed");
            }
            
            clientStates[DataSocketFD] = STATE_LOBBY;

            break;
        case STATE_LOBBY:
        
            strncpy(gameState.players[playerCount].username, RecvBuf,
            sizeof(gameState.players[0].username) - 1);
            playerFDS[playerCount] = DataSocketFD;
            playerCount++;
            snprintf(SendBuf, sizeof(SendBuf), "OK WELCOME %s", RecvBuf);
            l = strlen(SendBuf);
            n = write(DataSocketFD, SendBuf, l);
            clientStates[DataSocketFD] = STATE_PLAYING;
            break;
        case STATE_PLAYING:
            strncpy(SendBuf, "PLAYING GAME NOW s", sizeof(SendBuf)-1);
            SendBuf[sizeof(SendBuf)-1] = 0;
            l = strlen(SendBuf);
            n = write(DataSocketFD, SendBuf, l);
            break;
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
	res = select(FD_SETSIZE, &ReadFDs, NULL, NULL, &TimeVal);   
    //
	if (res < 0)
	{   FatalError("wait for input or timeout (select) failed");
	}
	if (res == 0)	/* timeout occurred */
	{
#ifdef DEBUG
	    printf("%s: Handling timeout...\n", Program);
#endif
	    PrintCurrentTime(); // If no client input in 250ms then just print to the servers own terminal 

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

        int result = ProcessRequest(i);
        if (result == 0)
        {   close(i);
            FD_CLR(i, &ActiveFDs);
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
