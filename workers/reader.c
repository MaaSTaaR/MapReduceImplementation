#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <signal.h>
#include <unistd.h>

#include "../core/server/server.h"

char resultLocation[ 255 ];

void *readFile( void *recvConnection )
{
	char requiredLocation[ 20 ], filePath[ 1000 ], clientMessage[ 500 ];
	int connection = ( intptr_t ) recvConnection;
	
	memset( requiredLocation, 0, 20 );
	
	recv( connection, requiredLocation, 20, 0 );
	
	clearCRLF( requiredLocation );
	
	sprintf( filePath, "%s%s", resultLocation, requiredLocation );
	
	printf( "\nLocation = %s\n", filePath );
	
	FILE *resFile = fopen( filePath, "r" );
	
	printf( "File Opened\n" );
	
	char *line = NULL;
	size_t bufsize = 0;
	
	while ( getline( &line, &bufsize, resFile ) != -1 )
	{
		//sleep( 7 );
		
		printf( "%s\n", line );
		
		sendMessage( connection, line );
		
		memset( clientMessage, 0, 500 );
		int messageSize = recv( connection, clientMessage, 500, 0 );
		
		if ( messageSize == 0 )
		{
			printf( "Connection Lost?\n" );
			fclose( resFile );
			close( connection );
			
			return NULL;
		}
		
		printf( "Recv Message = %s\n", clientMessage );
	}
	
	sendMessage( connection, "EOF" );
	
	printf( "End of File" );
	
	fclose( resFile );
	close( connection );
}

void newConnectionHandler( int connection )
{
	pthread_t readingThread;
	
	printf( "New Connection\n" );
	
	pthread_create( &readingThread, NULL, readFile, ( void * ) ( intptr_t ) connection );
}

// Source: http://stackoverflow.com/questions/18935446/program-received-signal-sigpipe-broken-pipe
void signal_callback_handler( int signum )
{
	printf("Caught signal SIGPIPE %d\n",signum);
}

int main( int argc, char **argv )
{
	if ( argc < 3 )
	{
		printf( "Worker ID (For Port) and Path ID (e.g. map5390 or reduce5500) Must Be Provided as Argument\n" );
		return 0;
	}
	
	// ... //
	
	char portBuffer[ 10 ];
	int port;
	
	sprintf( portBuffer, "540%s", argv[ 1 ] );
	
	port = atoi( portBuffer );
	
	printf( "Port = %d\n", port );
	
	// ... //
	
	strcpy( resultLocation, "/home/maastaar/proj3_res/" );
	strcat( resultLocation, argv[ 2 ] );
	strcat( resultLocation, "/" );
	
	printf( "Result Location = %s\n", resultLocation );
	
	// ... //
	
	// Source: http://stackoverflow.com/questions/18935446/program-received-signal-sigpipe-broken-pipe
	signal( SIGPIPE, signal_callback_handler );
	
	// ... //
	
	createServer( newConnectionHandler, port );
	
	return 0;
}
