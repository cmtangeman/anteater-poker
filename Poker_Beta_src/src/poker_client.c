/* ClockClient.c: simple interactive TCP/IP client for ClockServer
 * Author: Rainer Doemer, 5/15/23 (prior versions 2/16/15, 2/20/17)
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <netinet/in.h>
#include <netdb.h>

int gameLoop = 0;

/* #define DEBUG */	/* be verbose */

/*** global variables ****************************************************/

const char *Program	/* program name for descriptive diagnostics */
	= NULL;

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

int main(int argc, char *argv[])
{
    int l, n;
    int SocketFD,	/* socket file descriptor */
	PortNo;		/* port number */
    struct sockaddr_in
	ServerAddress;	/* server address we connect with */
    struct hostent
	*Server;	/* server host */
    char SendBuf[256];	/* message buffer for sending a message */
    char RecvBuf[256];	/* message buffer for receiving a response */

    Program = argv[0];	/* publish program name (for diagnostics) */
#ifdef DEBUG
    printf("%s: Starting...\n", argv[0]);
#endif
    if (argc < 3)
    {   fprintf(stderr, "Usage: %s hostname port\n", Program);
	exit(10);
    }
    Server = gethostbyname(argv[1]);
    if (Server == NULL)
    {   fprintf(stderr, "%s: no such host named '%s'\n", Program, argv[1]);
        exit(10);
    }
    PortNo = atoi(argv[2]);
    if (PortNo <= 2000)
    {   fprintf(stderr, "%s: invalid port number %d, should be >2000\n",
		Program, PortNo);
        exit(10);
    }
    ServerAddress.sin_family = AF_INET;	// 
    ServerAddress.sin_port = htons(PortNo);	// 
    ServerAddress.sin_addr = *(struct in_addr*)Server->h_addr_list[0];	// 



	// still in main, but before the do loop
	SocketFD = socket(AF_INET, SOCK_STREAM, 0);
	if (SocketFD < 0)
	{   FatalError("socket creation failed");
	}
	if (connect(SocketFD, (struct sockaddr*)&ServerAddress,
			sizeof(ServerAddress)) < 0)
	{   FatalError("connecting to server failed");
	}


	printf("%s: Enter a command to send to the anteater poker server! :\n"
		"         'START' to login to your anteater poker accounnt and start playing,\n"
		"         'BYE' to quit this client, or\n"
		"         'SHUTDOWN' to terminate the server\n"
		, argv[0]);


    do
    {	
	
	if(!gameLoop){

	printf("command: ");   // moved command prompt here, fixing reprompt issue
	// Send msg based off of initial promp first
	fgets(SendBuf, sizeof(SendBuf), stdin);
	l = strlen(SendBuf);
	if (SendBuf[l-1] == '\n')	// decrements and sets newline to null terminator so CHARLIE\n appears as just CHARLIE
	{   SendBuf[--l] = 0;
	}

	if (l == 0) {  // handle if user just inputed ENTER 
		continue;
	}
	if (0 == strcmp("BYE", SendBuf))
	{   break;	// Restart the Loop
	}
	if (l)	// if its more than 0. 
	{   

	    printf("%s: Sending message '%s'...\n", Program, SendBuf);
	    n = write(SocketFD, SendBuf, l);
	    if (n < 0)
	    {   FatalError("writing to socket failed");
	    }
		// Handle game state prompts 
	}
	}



	// Read the response 
	n = read(SocketFD, RecvBuf, sizeof(RecvBuf)-1);
	if (n < 0) 
	{   FatalError("reading from socket failed");
	}

	if (n == 0) {
		printf("%s: Server closed connection.\n", Program); // prevents "Recieved Resonse:" spam when server disconnection occurs
		break;
	}
	RecvBuf[n] = 0;
	printf("%s: Received response: %s\n", Program, RecvBuf);


	if (strstr(RecvBuf, "1)Check 2)Call 3)Bet 4)Fold 5)AllIn"))	// Haystack / needle function call
		{   // Client drives two step sequence for bet amount
		gameLoop = 1;
			printf("Your choice: ");
			fgets(SendBuf, sizeof(SendBuf), stdin);
			l = strlen(SendBuf);
			if (SendBuf[l-1] == '\n') SendBuf[--l] = 0;
			n = write(SocketFD, SendBuf, l);

			// if bet, also send amount 
			if (strcmp(SendBuf, "3") == 0)
			{   printf("Bet amount: ");
			// Fgets 
				fgets(SendBuf, sizeof(SendBuf), stdin);
				l = strlen(SendBuf);
				if (SendBuf[l-1] == '\n') SendBuf[--l] = 0;
				n = write(SocketFD, SendBuf, l);
			}
				/*
				n = read(SocketFD, RecvBuf, sizeof(RecvBuf)-1);
				if (n < 0) 
				{   FatalError("reading from socket failed");
				}
				RecvBuf[n] = 0;
				printf("%s: Received response: %s\n", Program, RecvBuf);
				*/

		}	
	// Loop back to send the response ->>

    } while(0 != strcmp("SHUTDOWN", SendBuf));
    printf("%s: Exiting...\n", Program);
	 close(SocketFD); // -> Keep the connection open! 
    return 0;
}

/* EOF poker_client.c */
