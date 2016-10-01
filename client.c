#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "core/server/server.h"

int myRecv( int connection, char *message )
{
	memset( message, 0, 1000 );
	int size = recv( connection, message, 1000 , 0 );
	clearCRLF( message );
	
	//printf( "%s", message );
	
	return size;
}

void main()
{
	for ( int i = 0; i < 50; i++ )
	{
		pid_t child = fork();
		
		if ( child == 0 ) {
		
		char masterHost[ 100 ] = "127.0.0.1", masterMessage[ 1000 ];
		int masterPort = 3539;
	
		char keywords[ 1 ][ 100 ] = { "Python" };
	
		// ... //
	
		struct sockaddr_in address;
		
		int connection = socket( PF_INET, SOCK_STREAM, 0 );
	
		address.sin_family = AF_INET;
		address.sin_port = htons( masterPort );
		address.sin_addr.s_addr = inet_addr( masterHost );
	
		memset( address.sin_zero, '\0', sizeof( address.sin_zero ) );
	
		// ... //
	
		int result = connect( connection, (struct sockaddr *) &address, sizeof( address ) );
	
		if ( result == -1 )
		{
			printf( "Cannot Connect to Master\n" );
			return;
		}
	
		printf( "Master: Connected\n" );
	
		// ... //
	
		sendMessage( connection, keywords[ 0 ] );
		myRecv( connection, masterMessage );
		sendMessage( connection, "[DONE!]" );
	
		time_t startedAt = time( NULL );
	
		while ( 1 )
		{
			int size = myRecv( connection, masterMessage );
		
			printf( "%s\n", masterMessage );
		
			if ( strcmp( masterMessage, "DONE" ) == 0 )
				break;
		}
	
		time_t endedAt = time( NULL );
	
		printf( "\n==== Total Time Elapsed = %d Sec. ====\n", ( endedAt - startedAt ) );
		}
	}
}
