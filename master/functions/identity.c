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

void *reduce( void *recvConnection )
{
	int numberOfItems;
	int connection = ( intptr_t ) recvConnection;
	char lineBuffer[ 2000 ];
	
	int prevMessageZero = 0, numerOfZeroMessages = 0;

	while ( 1 )
	{
		sendMessage( connection, "NEXT_ITEM" );
		
		memset( lineBuffer, 0, 2000 );
		int messageSize = recv( connection, lineBuffer, 2000 , 0 );
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
		
		//printf( "lineBuffer = %s\n", lineBuffer );
		
		if ( strcmp( lineBuffer, "EOL" ) == 0 )
			break;
		
		// ... //
	
		sendMessage( connection, lineBuffer );
	}
	
	printf( "----> Reduce Function Done\n" );
}

void newConnectionHandler( int connection )
{
	pthread_t reduceThread;
	
	printf( "New Connection\n" );
	
	pthread_create( &reduceThread, NULL, reduce, ( void * ) ( intptr_t ) connection );
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
	
	createServer( newConnectionHandler, 9402 );
	
	return 0;
}
