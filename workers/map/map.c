#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <time.h>
#include <signal.h>
#include <arpa/inet.h>

#include "../../core/server/server.h"
#include "../../core/search/search.h"
#include "cache.h"

#define NUMBER_OF_EXAMINED_LINES_TO_SEND_HEALTH_MESSAGE 4000

char dataFiles[ 4 ][ 255 ], resultLocation[ 255 ];
int currFileIdx, numberOfSearchRequests = 0;
pthread_mutex_t numberOfSearchRequestsMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t writeToCacheMutex = PTHREAD_MUTEX_INITIALIZER;

typedef struct search_info
{
	char **searchItems;
	int numberOfItems;
	FILE *resultsFile;
	int connection;
	int searchId;
	int listId;
	char *mapFunctionHost;
	int mapFunctionPort;
} search_info_t;

void initDataFiles()
{
	//strcpy( dataFiles[ 0 ], "/home/maastaar/fun/Univ/Master/525 - Advanced OS/Project I/data/1.xml" );
	//strcpy( dataFiles[ 1 ], "/home/maastaar/fun/Univ/Master/525 - Advanced OS/Project I/data/2.xml" );
	//strcpy( dataFiles[ 2 ], "/home/maastaar/fun/Univ/Master/525 - Advanced OS/Project I/data/3.xml" );
	//strcpy( dataFiles[ 3 ], "/home/maastaar/fun/Univ/Master/525 - Advanced OS/Project I/data/4.xml" );
	
	strcpy( dataFiles[ 0 ], "/home/maastaar/fun/Univ/Master/525 - Advanced OS/Project I/data/zhwiki-20160203-pages-articles-multistream.xml" );
	strcpy( dataFiles[ 1 ], "/home/maastaar/fun/Univ/Master/525 - Advanced OS/Project I/data/nlwiki-20160203-pages-articles-multistream.xml" );
	strcpy( dataFiles[ 2 ], "/home/maastaar/fun/Univ/Master/525 - Advanced OS/Project I/data/jawiki-20160203-pages-articles-multistream.xml" );
	strcpy( dataFiles[ 3 ], "/home/maastaar/fun/Univ/Master/525 - Advanced OS/Project I/data/commonswiki-20160203-pages-articles-multistream.xml" );
}

void map( char *line, FILE *resultsFile )
{
	fwrite( line, 1, strlen( line ), resultsFile );
}

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
		
		//printf( "    -> Recv Message (%d) %s\n", messageSize, clientMessage );
		
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

void sendItemsToMapFunction( int rfcConnection, char **searchItems, int itemsNumber )
{
	char serverMessage[ 1024 ];
	
	for ( int s = 0; s < itemsNumber; s++ )
	{
		if ( s != 0 )
		{
			while ( 1 )
			{
				memset( serverMessage, 0, 1024 );
				recv( rfcConnection, serverMessage, 1024, 0 );
				
				clearCRLF( serverMessage );
				
				if ( strcmp( serverMessage, "NEXT_WORD" ) != 0 )
					continue;
				else
					break;
			}
		}
		
		sendMessage( rfcConnection, searchItems[ s ] );
	}
	
	memset( serverMessage, 0, 1024 );
	
	recv( rfcConnection, serverMessage, 1024, 0 );
	
	sendMessage( rfcConnection, "[DONE!]\n" );
}

void *search( void *argSearchInfo, int cacheHit, int mustBeCached, ref_node_t *currWord ) //( char **searchItems, int numberOfItems, FILE *resultsFile )
{
	search_info_t *searchInfo = (search_info_t *) argSearchInfo;
	char toBeSentLocation[ 255 ], clientMessage[ 10000 ], *line = NULL;
	size_t bufsize = 0;
	int numberOfExaminedLines = 0, numberOfResults = 0;
	
	if ( cacheHit == 0 )
	{
		// ... //
		
		// Connecting to Map Function
/****
		struct sockaddr_in address;
		
		int rfcConnection = socket( PF_INET, SOCK_STREAM, 0 );
	
		address.sin_family = AF_INET;
		address.sin_port = htons( searchInfo->mapFunctionPort );
		address.sin_addr.s_addr = inet_addr( searchInfo->mapFunctionHost );
	
		memset( address.sin_zero, '\0', sizeof( address.sin_zero ) );
	
		// ... //
	
		int result = connect( rfcConnection, (struct sockaddr *) &address, sizeof( address ) );
	
		if ( result == -1 )
		{
			printf( "Cannot Connect to Map Function\n" );
			return NULL;
		}
		
		printf( "Map Function: Connected\n" );
		
		sendItemsToMapFunction( rfcConnection, searchInfo->searchItems, searchInfo->numberOfItems );
		****/
		// ... //
		
		FILE *currFile = fopen( dataFiles[ searchInfo->listId ], "r" );

		while ( getline( &line, &bufsize, currFile ) != -1 )
		{
			//printf( "(%d)", numberOfExaminedLines );
			
			if ( ( numberOfExaminedLines % NUMBER_OF_EXAMINED_LINES_TO_SEND_HEALTH_MESSAGE ) == 0 )
				sendMessage( searchInfo->connection, "I_AM_ALIVE" );
			
			// ... //
			
			//printf( "Waiting For Map Function Message\n" );
			
			/****
			memset( clientMessage, 0, 10000 );
			recv( rfcConnection, clientMessage , 10000 , 0 );
			
			//printf( "    -> Client Message = %s\n", clientMessage );
			
			if ( strcmp( clientMessage, "LIST_MEMBER" ) == 0 )
			{
				sendMessage( rfcConnection, line );
				
				memset( clientMessage, 0, 10000 );
				recv( rfcConnection, clientMessage , 10000 , 0 );	
				
				if ( strcmp( clientMessage, "[NONE]" ) != 0 )
				{
					fwrite( clientMessage, 1, strlen( clientMessage ), searchInfo->resultsFile );
					
					numberOfResults++;
				}
			}
			****/
			
			if ( isResult( searchInfo->searchItems, searchInfo->numberOfItems, line ) )
			{
				map( line, searchInfo->resultsFile );
			
				numberOfResults++;
			}
		
			// ... //
		
			// Must recieve "CONTINUE"
			if ( ( numberOfExaminedLines % NUMBER_OF_EXAMINED_LINES_TO_SEND_HEALTH_MESSAGE ) == 0 )
			{
				memset( clientMessage, 0, 10000 );
				recv( searchInfo->connection, clientMessage , 10000 , 0 );
			}
		
			// ... //
		
			numberOfExaminedLines++;
		}
		
		////sendMessage( rfcConnection, "EOL" );
		
		fclose( currFile );
	}
	else
	{
		for ( int r = 0; r < currWord->numberOfCachedItems; r++ )
		{
			if ( ( numberOfExaminedLines % NUMBER_OF_EXAMINED_LINES_TO_SEND_HEALTH_MESSAGE ) == 0 )
				sendMessage( searchInfo->connection, "I_AM_ALIVE" );
			
			// ... //
			
			fwrite( currWord->cache[ r ], 1, strlen( currWord->cache[ r ] ), searchInfo->resultsFile );
			
			// ... //
			
			// Must recieve "CONTINUE"
			if ( ( numberOfExaminedLines % NUMBER_OF_EXAMINED_LINES_TO_SEND_HEALTH_MESSAGE ) == 0 )
			{
				memset( clientMessage, 0, 10000 );
				recv( searchInfo->connection, clientMessage , 10000 , 0 );
			}
		
			// ... //
		
			numberOfExaminedLines++;
		}
	}
	
	// ... //
	
	sprintf( toBeSentLocation, "%d", searchInfo->searchId );
	
	// ... //
	
	if ( mustBeCached == 1 )
	{
		char *line = NULL;
		size_t bufsize = 0;
		int s = 0;
		
		// ... //
		
		printf( "Caching Started\n" );
		
		// ... //
		
		currWord->cache = malloc( sizeof( char * ) * numberOfResults );
				
		rewind( searchInfo->resultsFile );
		
		while ( getline( &line, &bufsize, searchInfo->resultsFile ) != -1 )
		{
			currWord->cache[ s ] = malloc( sizeof( char ) * ( strlen( line ) * 2 ) );
			
			strcpy( currWord->cache[ s ], line );
			
			//printf( "%s Cached\n", currWord->cache[ s ] );
			
			s++;
		}
		
		currWord->numberOfCachedItems = s;
	}
	
	// ... //
	
	sendMessage( searchInfo->connection, "SEARCH_DONE" );
	
	memset( clientMessage, 0, 10000 );
		
	recv( searchInfo->connection, clientMessage , 10000 , 0 );
		
	clearCRLF( clientMessage );
	
	if ( strcmp( clientMessage, "LOCATION" ) == 0 )
		sendMessage( searchInfo->connection, toBeSentLocation ); // Result location which will be used by reduce worker
	
	// ... //
	
	fclose( searchInfo->resultsFile );
}

void *newSearchRequest( void *recvConnection )
{
	FILE *resultsFile;
	int numberOfItems = 0;
	pthread_t searchThread;
	char clientMessage[ 2000 ];
	search_info_t *currSearchInfo = malloc( sizeof( search_info_t ) );
	
	int connection = ( intptr_t ) recvConnection;
	
	pthread_mutex_lock( &numberOfSearchRequestsMutex );
	numberOfSearchRequests++;
	
	currSearchInfo->searchId = numberOfSearchRequests;
	pthread_mutex_unlock( &numberOfSearchRequestsMutex );
	
	
	printf( "    -> New Search Request (%d). Waiting Items\n", currSearchInfo->searchId );
	
	sendMessage( connection, "OK" );
	
	// ... //
	
	memset( clientMessage, 0, 2000 );
	recv( connection, clientMessage , 2000 , 0 );
	clearCRLF( clientMessage );
	
	currSearchInfo->listId = atoi( clientMessage ) - 1;
	
	printf( "List ID = %d\n", currSearchInfo->listId );
	
	// ... //
	
	sendMessage( connection, "OK" );
	
	// ... //
	
	memset( clientMessage, 0, 2000 );
	recv( connection, clientMessage , 2000 , 0 );
	clearCRLF( clientMessage );
	
	currSearchInfo->mapFunctionHost = malloc( sizeof( char ) * strlen( clientMessage ) );
	
	strcpy( currSearchInfo->mapFunctionHost, clientMessage );
	
	printf( "Map Function Host = %s\n", currSearchInfo->mapFunctionHost );
	
	// ... //
	
	sendMessage( connection, "OK" );
	
	// ... //
	
	memset( clientMessage, 0, 2000 );
	recv( connection, clientMessage , 2000 , 0 );
	clearCRLF( clientMessage );
	
	currSearchInfo->mapFunctionPort = atoi( clientMessage );
	
	printf( "Map Function Port = %d\n", currSearchInfo->mapFunctionPort );
	
	// ... //
	
	sendMessage( connection, "OK" );
	
	// ... //
	
	currSearchInfo->searchItems = recieveItems( connection, &( currSearchInfo->numberOfItems ) );
	
	// ... //
	
	ref_node_t *wordRefs = NULL;
	int cacheHit = 0, mustBeCached = 0;
	
	if ( currSearchInfo->numberOfItems == 1 )
	{
		pthread_mutex_lock( &writeToCacheMutex );
		
		printf( "Candidate For Cache (%s)\n", currSearchInfo->searchItems[ 0 ] );
		
		wordRefs = findWordCache( currSearchInfo->searchItems[ 0 ], currSearchInfo->listId );
		
		if ( wordRefs == NULL )
			wordRefs = newReference( currSearchInfo->searchItems[ 0 ], currSearchInfo->listId );
		
		if ( wordRefs->cache != NULL )
		{
			printf( "A Pretty Cache HIT\n" );
			
			cacheHit = 1;
		}
		else
		{
			if ( wordRefs->refCount <= NUMBER_OF_OCC_BEFORE_CACHE )
			{
				printf( "Just Incerement the Counter\n" );
				
				incrementCounter( wordRefs );
			}
			else
			{
				printf( "Must Be Cached\n" );
				
				mustBeCached = 1;
			}
		}
		
		pthread_mutex_unlock( &writeToCacheMutex );
	}
	
	// ... //
	
	char resultPath[ 255 ];
	
	sprintf( resultPath, "%s%d", resultLocation, currSearchInfo->searchId );
	
	printf( "Result file will be in %s\n", resultPath );
	
	currSearchInfo->connection = connection;
	currSearchInfo->resultsFile = fopen( resultPath, "w+" );
	
	// ... //
	
	time_t searchStartedAt = time( NULL );
	
	search( currSearchInfo, cacheHit, mustBeCached, wordRefs );
	
	time_t searchEndeddAt = time( NULL );
	
	printf( "Search Elapsed in %d seconds\n", ( searchEndeddAt - searchStartedAt ) );
}

void newConnectionHandler( int connection )
{
	char clientMessage[ 2000 ];
	
	printf( "    -> New Connection\n" );

	memset( clientMessage, 0, 2000 );
	recv( connection, clientMessage , 2000 , 0 );
	clearCRLF( clientMessage );
	
	// ... //
		
	if ( strcmp( clientMessage, "SEARCH_REQ" ) == 0 )
	{
		pthread_t searchThread;
		
		pthread_create( &searchThread, NULL, newSearchRequest, ( void * ) ( intptr_t ) connection );
	}
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
		printf( "File Index Must Be Provided as Argument\n" );
		return 0;
	}
	
	initDataFiles();
	
	// ... //
	
	currFileIdx = atoi( argv[ 1 ] ) - 1;
		
	// ... //
	
	char portBuffer[ 10 ];
	int port;
	
	sprintf( portBuffer, "539%d", currFileIdx );
	
	if ( argc == 2 )
		port = atoi( portBuffer );
	else if ( argc == 3 )
	{
		strcpy( portBuffer, argv[ 2 ] );
		
		port = atoi( argv[ 2 ] );
	}
	
	printf( "Port = %d\n", port );
	
	// ... //
	
	strcpy( resultLocation, "/home/maastaar/proj3_res/map" );
	strcat( resultLocation, portBuffer );
	strcat( resultLocation, "/" );
	
	printf( "Result Location = %s\n", resultLocation );
	
	// ... //
	
	initCache();
	
	// ... //
	
	// Source: http://stackoverflow.com/questions/18935446/program-received-signal-sigpipe-broken-pipe
	signal( SIGPIPE, signal_callback_handler );
	
	// ... //
	
	createServer( newConnectionHandler, port );
	
	return 0;
}
