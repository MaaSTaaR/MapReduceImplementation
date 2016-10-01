#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <signal.h>

#include "../../core/server/server.h"

#define NUMBER_OF_EXAMINED_LINES_TO_SEND_HEALTH_MESSAGE 500

char resultLocation[ 255 ];
int numberOfReduces = 0;
pthread_mutex_t numberOfReducesMutex = PTHREAD_MUTEX_INITIALIZER;

void getReaderInformation( int connection, char *host, int *port, char *location )
{
	char portBuffer[ 20 ];
	
	memset( host, 0, 1000 );
	recv( connection, host, 1000 , 0 );
	clearCRLF( host );
	
	printf( "    -> Host = %s\n", host );
	
	// ... //
	
	sendMessage( connection, "PORT" );
	
	printf( "Port Message Sent\n" );
	
	memset( portBuffer, 0, 20 );
	recv( connection, portBuffer, 20 , 0 );
	clearCRLF( host );
	
	printf( "    -> Port = %s\n", portBuffer );
	
	*port = atoi( portBuffer );
	
	// ... //
	
	sendMessage( connection, "LOCATION" );
	
	memset( location, 0, 20 );
	recv( connection, location, 20 , 0 );
	clearCRLF( host );
	
	printf( "    -> Location = %s\n", location );
}

int connectToReader( char *host, int port )
{
	struct sockaddr_in address;
	
	int readerConnection = socket( PF_INET, SOCK_STREAM, 0 );
	
	address.sin_family = AF_INET;
	address.sin_port = htons( port );
	address.sin_addr.s_addr = inet_addr( host );
	
	memset( address.sin_zero, '\0', sizeof( address.sin_zero ) );
	
	int result = connect( readerConnection, (struct sockaddr *) &address, sizeof( address ) );
	
	if ( result == -1 )
	{
		printf( "Cannot connect to %s:%d\n", host, port );
		exit( -1 );
	}
	
	printf( "Connected %d\n", result );
	
	return readerConnection;
}

void reduce( char *line, FILE *resultFile )
{
	size_t ret = fwrite( line, 1, strlen( line ), resultFile );
	fwrite( "\n", 1, strlen( "\n" ), resultFile );
	
	//printf( "Line written (%d) = %s\n", ret, line );
}

void *reduceRequest( void *recvConnection )
{
	char readerHost[ 1000 ], mapResultLocation[ 20 ], resultPath[ 255 ], lineBuffer[ 2000 ], reduceFunctionHost[ 1000 ], reduceFunctionPortBuffer[ 1000 ];
	int readerPort, readerConnection, connection = ( intptr_t ) recvConnection, numberOfExaminedLines = 0, reduceFunctionPort;
	
	getReaderInformation( connection, readerHost, &readerPort, mapResultLocation );
	
	// ... //
	
	// Get Reduce Function info
	
	sendMessage( connection, "REDUCE_FUNCTION_HOST" );
	
	memset( reduceFunctionHost, 0, 1000 );
	recv( connection, reduceFunctionHost, 1000 , 0 );
	clearCRLF( reduceFunctionHost );
	
	printf( "    -> reduceFunctionHost = %s\n", reduceFunctionHost );
	
	// ... //
	
	sendMessage( connection, "REDUCE_FUNCTION_PORT" );
	
	memset( reduceFunctionPortBuffer, 0, 1000 );
	recv( connection, reduceFunctionPortBuffer, 1000 , 0 );
	clearCRLF( reduceFunctionPortBuffer );
	
	reduceFunctionPort = atoi( reduceFunctionPortBuffer );
	
	printf( "    -> reduceFunctionPort = %d\n", reduceFunctionPort );
	
	// ... //
	
	readerConnection = connectToReader( readerHost, readerPort );
		
	// ... //
	
	pthread_mutex_lock( &numberOfReducesMutex );
	
	numberOfReduces++;
	
	int reduceId = numberOfReduces;
	
	pthread_mutex_unlock( &numberOfReducesMutex );
	
	// ... //
	
	sprintf( resultPath, "%s%d", resultLocation, reduceId );
	
	printf( "Result Path = %s\n", resultPath );
	
	FILE *resultFile = fopen( resultPath, "w+" );
	
	// ... //
	
	printf( "Connected To Reader\n" );
	
	sendMessage( readerConnection, mapResultLocation );
	
	// ... //
	
	// Connecting to Reuce Function
	/****
	struct sockaddr_in address;
		
	int rfcConnection = socket( PF_INET, SOCK_STREAM, 0 );
	
	address.sin_family = AF_INET;
	address.sin_port = htons( reduceFunctionPort );
	address.sin_addr.s_addr = inet_addr( reduceFunctionHost );
	
	memset( address.sin_zero, '\0', sizeof( address.sin_zero ) );
	
	// ... //
	
	int result = connect( rfcConnection, (struct sockaddr *) &address, sizeof( address ) );
	
	if ( result == -1 )
	{
		printf( "Cannot Connect to Reduce Function\n" );
		return NULL;
	}
	
	printf( "Reduce Function: Connected\n" );
	****/

	
	// ... //
	
	char reduceFunctionBuffer[ 10000 ];
	
	while ( 1 )
	{
		/*
		// For Testing Fault-Tolerance
		if ( numberOfExaminedLines == 1 )
		{
			fclose( resultFile );
			close( readerConnection );
			
			_exit( 0 );
		}*/
			
		if ( ( numberOfExaminedLines % NUMBER_OF_EXAMINED_LINES_TO_SEND_HEALTH_MESSAGE ) == 0 )
			sendMessage( connection, "I_AM_ALIVE" );
		
		memset( lineBuffer, 0, 2000 );
		int messageSize = recv( readerConnection, lineBuffer, 2000 , 0 );
		clearCRLF( lineBuffer );
		
		if ( strcmp( lineBuffer, "EOF" ) != 0 )
		{
			
			/****
			memset( reduceFunctionBuffer, 0, 10000 );
			recv( rfcConnection, reduceFunctionBuffer, 10000 , 0 );
			clearCRLF( reduceFunctionBuffer );
			
			printf( "    reduceFunctionBuffer -> %s\n", reduceFunctionBuffer );
			
			if ( strcmp( reduceFunctionBuffer, "NEXT_ITEM" ) == 0 )
			{
				sendMessage( rfcConnection, lineBuffer );
				
				memset( reduceFunctionBuffer, 0, 10000 );
				recv( rfcConnection, reduceFunctionBuffer, 10000 , 0 );
				clearCRLF( reduceFunctionBuffer );
				
				printf( "Recieved Reduce Function Value = %s\n", reduceFunctionBuffer );
				
				fwrite( reduceFunctionBuffer, 1, strlen( reduceFunctionBuffer ), resultFile );
				fwrite( "\n", 1, strlen( "\n" ), resultFile );
				
				printf( "Line Written to Result File = %s\n", reduceFunctionBuffer );
			}
			****/
			
			 reduce( lineBuffer, resultFile );
		}
		else
			break;
		
		sendMessage( readerConnection, "NEXT_LINE" );
		
		numberOfExaminedLines++;
	}
	
	////sendMessage( rfcConnection, "EOL" );
	
	// ... //
	
	sendMessage( connection, "REDUCE_DONE" );
	
	// ... //
	
	char clientMessage[ 500 ], location[ 100 ];
	
	memset( clientMessage, 0, 500 );
	recv( connection, clientMessage, 500 , 0 );
	
	sprintf( location, "%d", reduceId );
	
	sendMessage( connection, location );
	
	// ... //
	
	fclose( resultFile );
	close( readerConnection );
}

void newConnectionHandler( int connection )
{
	pthread_t reduceThread;
	
	printf( "    -> New Connection\n" );
	
	pthread_create( &reduceThread, NULL, reduceRequest, ( void * ) ( intptr_t ) connection );
}

// Source: http://stackoverflow.com/questions/18935446/program-received-signal-sigpipe-broken-pipe
void signal_callback_handler( int signum )
{
	printf("Caught signal SIGPIPE %d\n",signum);
}

int main( int argc, char **argv )
{
	if ( argc == 1 )
	{
		printf( "Worker ID Must Be Provided as Argument\n" );
		return 0;
	}
	
	// ... //
	
	char portBuffer[ 10 ];
	int port;
	
	sprintf( portBuffer, "550%s", argv[ 1 ] );
	
	if ( argc == 2 )
		port = atoi( portBuffer );
	else if ( argc == 3 )
	{
		strcpy( portBuffer, argv[ 2 ] );
		
		port = atoi( argv[ 2 ] );
	}
	
	printf( "Port = %d\n", port );
	
	// ... //
	
	strcpy( resultLocation, "/home/maastaar/proj3_res/reduce" );
	strcat( resultLocation, portBuffer);
	strcat( resultLocation, "/" );
	
	printf( "Result Location = %s\n", resultLocation );
	
	// ... //
	
	// Source: http://stackoverflow.com/questions/18935446/program-received-signal-sigpipe-broken-pipe
	signal( SIGPIPE, signal_callback_handler );
	
	// ... //
	
	createServer( newConnectionHandler, port );
	
	/*strcpy( host, "127.0.0.1" );
	port = 5400;
	strcpy( location, "1" );
	
	newConnectionHandler( -1 );*/
	
	return 0;
}
