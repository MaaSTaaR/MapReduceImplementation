#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>

void clearCRLF( char *string )
{
	int size = 0;
	
	while ( *string != '\0' )
	{
		if ( *string == '\n' || *string == '\r' )
		{
			*string = '\0';
			break;
		}
		else
		{
			size++;
		}
		
		string++;
	}
}

void sendMessage( int connection, char *message )
{
	write( connection, message, strlen( message ) );
}

void createServer( void ( *requestHandler )( int connection ), int port )
{
	struct sockaddr_in server, client;	
	int addressSize = sizeof( struct sockaddr_in );

	
	printf( "Initializing ...\n" );
	
	int serverSocket = socket( AF_INET , SOCK_STREAM , 0 );
	
	if ( serverSocket == -1 )
	{
		printf( "Could not create socket\n" );
		return;
	}
	
	printf( "Socket Created\n" );
	
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_port = htons( port );
	
	if ( bind( serverSocket, ( struct sockaddr * ) &server , sizeof( server ) ) < 0 )
	{
		// Quick and Dirty
		if ( errno == 98 )
		{
			port = 5391;
			
			server.sin_port = htons( port );
			
			if ( bind( serverSocket, ( struct sockaddr * ) &server , sizeof( server ) ) < 0 )
			{
					printf( "Could not bind %d\n", errno );
					return;
			}
		}
	}
	
	printf( "Server Bound. %d\n", port );
	
	listen( serverSocket, 3 );
	
	// ... //
	
	while ( 1 )
	{
		int connection = accept( serverSocket, (struct sockaddr *) &client, (socklen_t*) &addressSize );
	
		if ( connection < 0 )
		{
			printf( "Accept Failed\n" );
			return;
		}
	
		printf( "Connection Recieved\n" );
	
		requestHandler( connection );
	}
}
