#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <signal.h>
#include <unistd.h>

#include "../../core/server/server.h"
#include "../../core/search/search.h"

char **recieveItems( int connection, int *numberOfItems )
{
	char message[ 1000 ], clientMessage[ 2000 ], **searchItems, **originalSearchItems;
  	int messageSize;
  	
  	searchItems = malloc( 1000 * sizeof( char * ) );
	
	originalSearchItems = searchItems;
	
	int currItemIdx = 0;
		
	while ( 1 )
	{
		memset( clientMessage, 0, 2000 );
		messageSize = recv( connection , clientMessage , 2000 , 0 );
		
		if ( messageSize <= 0 )
			continue;
		
		clearCRLF( clientMessage );
		
		printf( "Recevied = %s\n", clientMessage );
		
		if ( strcmp( clientMessage, "[DONE!]" ) != 0 )
		{
			char *currWord = malloc( strlen( clientMessage ) * sizeof( char ) );
			
			strcpy( currWord, clientMessage );
			
			*searchItems = currWord;
			
			sendMessage( connection, "NEXT_WORD" );
			
			searchItems++;
			currItemIdx++;
		}
		else
		{
			break;
		}
		
		//memset( clientMessage, 0, 2000 );
	}
	
	searchItems = originalSearchItems;
	
	*numberOfItems = currItemIdx;
	
	return searchItems;
}

void *map( void *recvConnection )
{
	int numberOfItems;
	int connection = ( intptr_t ) recvConnection;
	char lineBuffer[ 10000 ];
	
	char **searchItems = recieveItems( connection, &numberOfItems );
	
	printf( "Receiving Items Done\n" );
	
	// ... //
	
	int prevMessageZero = 0, numerOfZeroMessages = 0;
	
	while ( 1 )
	{
		sendMessage( connection, "LIST_MEMBER" );
		
		memset( lineBuffer, 0, 10000 );
		int messageSize = recv( connection, lineBuffer, 10000 , 0 );
		//clearCRLF( lineBuffer );
		
		if ( messageSize <= 0 )
		{
			if ( prevMessageZero == 1 && numerOfZeroMessages == 500 )
			{
				printf( "Connection Lost\n" );
				close( connection );
				
				return NULL;
			}
			else if ( prevMessageZero == 0 )
			{
				prevMessageZero = 1;
				numerOfZeroMessages--;
			}
			else if ( prevMessageZero == 1 )
			{
				numerOfZeroMessages++;
			}
		}
		else
		{
			prevMessageZero = 0;
		}
		
		//printf( "===== MAP Function (%d), lineBuffer = %s\n", messageSize, lineBuffer );
		
		if ( strcmp( lineBuffer, "EOL" ) == 0 )
			break;
		
		// ... //
	
		if ( isResult( searchItems, numberOfItems, lineBuffer ) )
			sendMessage( connection, lineBuffer );
		else
			sendMessage( connection, "[NONE]" );
	}
	
	printf( "    --> Map Function Done!\n" );
	
	close( connection );
}

void newConnectionHandler( int connection )
{
	pthread_t mapThread;
	
	printf( "New Connection\n" );
	
	pthread_create( &mapThread, NULL, map, ( void * ) ( intptr_t ) connection );
}

// Source: http://stackoverflow.com/questions/18935446/program-received-signal-sigpipe-broken-pipe
void signal_callback_handler( int signum )
{
	printf("Caught signal SIGPIPE %d\n",signum);
}

int main( int argc, char **argv )
{
	// ... //
	
	// Source: http://stackoverflow.com/questions/18935446/program-received-signal-sigpipe-broken-pipe
	signal( SIGPIPE, signal_callback_handler );
	
	// ... //
	
	createServer( newConnectionHandler, 9401 );
	
	return 0;
}
