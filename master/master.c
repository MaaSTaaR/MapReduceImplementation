#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>
#include <errno.h>
#include  <sys/types.h>
#include  <sys/wait.h>

#include "search_list.h"
#include "../core/server/server.h"

#define NUMBER_OF_AVAILABLE_FILES 3

#define MAP_FUNCTION_HOST "127.0.0.1"
#define MAP_FUNCTION_PORT "9401"

#define REDUCE_FUNCTION_HOST "127.0.0.1"
#define REDUCE_FUNCTION_PORT "9402"

typedef struct scheduler_parameters
{
	worker_t *currWorker;
	search_node_t *currSearch;
	int fileId;
	int client;
} scheduler_parameters_t;

pthread_mutex_t 	findReduceWorkerMutex = PTHREAD_MUTEX_INITIALIZER, 
					addResultMutex = PTHREAD_MUTEX_INITIALIZER,
					findMapWorkerMutex = PTHREAD_MUTEX_INITIALIZER,
					newConnectionMutex = PTHREAD_MUTEX_INITIALIZER;

void *schedule( void *recvCurrWorker );
worker_t *findBackup( worker_t *currWorker );
void connectToReduceWorker( worker_t *currReduceWorker, char *readerHost, char *readerPort, char *resultLocation, search_node_t *currSearch );

void printWorker( worker_t *currWorker )
{
	printf( "%s:%d -> %d\n", currWorker->host, currWorker->port, currWorker->state );
	
	foreach( currWorker->backup, printWorker );
}

char **initSearchItems()
{
	char **searchItems;
  	
  	searchItems = malloc( 1000 * sizeof( char * ) );
  	
  	return searchItems;
}

char **getItems( int *itemsNumber )
{
	char **searchItems = initSearchItems(), **orgSearchItems;
	char *currWord;
	
	*itemsNumber = 0;
	
  	orgSearchItems = searchItems;
  	
	printf( "Please Enter Search Items. Write END When You're Done\n" );
	
	while ( 1 )
	{
		char wordBuffer[ 1000 ];
		
		scanf( "%s", wordBuffer );
		
		// ... //
		
		if ( strcmp( wordBuffer, "END" ) == 0 )
			break;
		
		// ... //
		
		currWord = malloc( strlen( wordBuffer ) * sizeof( char ) );
			
		strcpy( currWord, wordBuffer );
			
		*searchItems = currWord;
			
		searchItems++;
		( *itemsNumber )++;
	}
	
	return orgSearchItems;
}

worker_t *connectToWorker( worker_t *currWorker )
{
	struct sockaddr_in address;
	struct timeval timeout;
	
	//pthread_mutex_lock( &newConnectionMutex );
	
	currWorker->connection = socket( PF_INET, SOCK_STREAM, 0 );
	
	printf( "    connectToWorker (%s:%d) -> Connection = %d\n", currWorker->host, currWorker->port, currWorker->connection );
	
	address.sin_family = AF_INET;
	address.sin_port = htons( currWorker->port );
	address.sin_addr.s_addr = inet_addr( currWorker->host );
	
	memset( address.sin_zero, '\0', sizeof( address.sin_zero ) );
	
	// ... //
	
	int result = connect( currWorker->connection, (struct sockaddr *) &address, sizeof( address ) );
	
	if ( result != -1 )
		currWorker->connection_state = CONNECTED;
	else
	{
		printf( "Cannot connect to %s:%d\n", currWorker->host, currWorker->port );
		
		currWorker->state = STATE_DEAD;
		
		worker_t *backup = findBackup( currWorker );
			
		if ( backup == NULL )
		{
			printf( "No Backup Found for %s:%d\n", currWorker->host, currWorker->port );
			
			//pthread_mutex_unlock( &newConnectionMutex );
			
			while ( 1 );
			//exit( -1 );
		}
		
		//pthread_mutex_unlock( &newConnectionMutex );
		
		return connectToWorker( backup );
	}
	
	//pthread_mutex_unlock( &newConnectionMutex );
	
	return currWorker;
}

worker_t *getIdleReduceWorker()
{
	//pthread_mutex_lock( &findReduceWorkerMutex );
	
	worker_t *currWorker = reduceWorkers->head;
	
	while ( currWorker != NULL )
	{
		if ( currWorker->state == STATE_IDLE )
		{
			currWorker->state = STATE_WORKING;
			
			//pthread_mutex_unlock( &findReduceWorkerMutex );
			
			return currWorker;
		}
		
		currWorker = currWorker->next;
	}
	
	//pthread_mutex_unlock( &findReduceWorkerMutex );
}

void sendItemsToMapWorker( worker_t *currWorker, search_node_t *currSearch, int fileId )
{
	char serverMessage[ 1024 ];
	
	currWorker->state = STATE_WORKING;
	
	// ... //
	
	sendMessage( currWorker->connection, "SEARCH_REQ\n" );
	
	memset( serverMessage, 0, 1024 );
	recv( currWorker->connection, serverMessage, 1024, 0 );
	
	// ... //
	
	char fileIdBuffer[ 20 ];
	
	sprintf( fileIdBuffer, "%d", fileId );
	
	sendMessage( currWorker->connection, fileIdBuffer );
	
	// ... //
	
	memset( serverMessage, 0, 1024 );
	recv( currWorker->connection, serverMessage, 1024, 0 );
	
	// ... //
	
	sendMessage( currWorker->connection, MAP_FUNCTION_HOST );
	
	// ... //
	
	memset( serverMessage, 0, 1024 );
	recv( currWorker->connection, serverMessage, 1024, 0 );
	
	// ... //
	
	sendMessage( currWorker->connection, MAP_FUNCTION_PORT );
	
	// ... //
	
	memset( serverMessage, 0, 1024 );
	recv( currWorker->connection, serverMessage, 1024, 0 );
	
	// ... //
	
	for ( int s = 0; s < currSearch->itemsNumber; s++ )
	{
		if ( s != 0 )
		{
			while ( 1 )
			{
				memset( serverMessage, 0, 1024 );
				recv( currWorker->connection, serverMessage, 1024, 0 );
				
				clearCRLF( serverMessage );
				
				if ( strcmp( serverMessage, "NEXT_WORD" ) != 0 )
					continue;
				else
					break;
			}
		}
		
		sendMessage( currWorker->connection, currSearch->items[ s ] );
	}
	
	memset( serverMessage, 0, 1024 );
	
	recv( currWorker->connection, serverMessage, 1024, 0 );
	
	sendMessage( currWorker->connection, "[DONE!]\n" );
}

worker_t *findBackup( worker_t *currWorker )
{
	worker_t *currBackupWorker = ( currWorker->type == WORKER_TYPE_MAP ) ? mapWorkers->head : reduceWorkers->head;
		
	while ( currBackupWorker != NULL )
	{
		if ( currBackupWorker->state != STATE_DEAD )
			return currBackupWorker;
		
		currBackupWorker = currBackupWorker->next;
	}
	
	return currBackupWorker;
}

int listenToWorker( worker_t *currWorker, search_node_t *currSearch, scheduler_parameters_t *currScheduleParameters  )
{
	char serverMessage[ 1024 ];
	
	while ( 1 )
	{
		//printf( "listenToWorker: Waiting for Map Worker\n" );
		
		memset( serverMessage, 0, 1024 );
		
		int recvRes = recv( currWorker->connection, serverMessage, 1024, 0 );
		
		//printf( "listenToWorker %s:%d: recieved message = %s\n", currWorker->host, currWorker->port, serverMessage );
		
		if ( recvRes == 0 )
		{
			close( currWorker->connection );
			
			currWorker->state = STATE_DEAD;
			currWorker->connection_state = DISCONNECTED;
			
			// ... //
			
			worker_t *backup = findBackup( currWorker );
			
			if ( backup == NULL )
			{
				printf( "\t\t\t === listenToWorker: No Backup Found for %s:%d ===\n", currWorker->host, currWorker->port );
				exit( -1 );
			}
			
			// ... //
			
			if ( currWorker->type == WORKER_TYPE_MAP )
			{
				printf( "\t\t==== Backup Found for Map Worker (%s:%d). The Backup is %s:%d\n", currWorker->host, currWorker->port, backup->host, backup->port );
				
				// ... //
				
				currScheduleParameters->currWorker = backup;
				
				// ... //
		
				schedule( ( void * ) currScheduleParameters );
				
				printf( "\t\t==== Backup schedule call done!\n" );
			}
			else if ( currWorker->type == WORKER_TYPE_REDUCE )
			{
				printf( "Backup Found for Reduce Worker (%s:%d). The Backup is %s:%d\n", currWorker->host, currWorker->port, backup->host, backup->port );
				
				connectToReduceWorker( backup, currWorker->reduce_info->readerHost, currWorker->reduce_info->readerPort, currWorker->reduce_info->resultLocation, currSearch );
				
				close( currWorker->connection );
			}
			
			return 0;
		}
		else
		{
			if ( strcmp( serverMessage, "I_AM_ALIVE" ) == 0 ) // Good. The worker is working fine.
			{
				sendMessage( currWorker->connection, "CONTINUE" );
				continue;
			}
			else if ( strcmp( serverMessage, "SEARCH_DONE" ) == 0 || strcmp( serverMessage, "REDUCE_DONE" ) == 0 )
			{
				if ( currWorker->type == WORKER_TYPE_REDUCE )
				{
					////printf( "==== Reduce Done ====\n" );
					
					char reduceResultLocation[ 1024 ];
					
					sendMessage( currWorker->connection, "LOCATION" );
					
					memset( reduceResultLocation, 0, 1024 );
					recv( currWorker->connection, reduceResultLocation, 1024, 0 );
					
					//pthread_mutex_lock( &addResultMutex );
					addNewResult( currSearch->resultList, currWorker, reduceResultLocation );
					//pthread_mutex_unlock( &addResultMutex );
					
					close( currWorker->connection );
					
					currWorker->state = STATE_IDLE;
				}
				else
				{
					printf( "==== Search Done By %s:%d ====\n", currWorker->host, currWorker->port );
				}
				
				break;
			}
		}
	}
	
	return 1;
}

void connectToReduceWorker( worker_t *currReduceWorker, char *readerHost, char *readerPort, char *resultLocation, search_node_t *currSearch )
{
	char serverMessage[ 1024 ];
	
	currReduceWorker->state = STATE_WORKING;
	
	// ... //
	
	connectToWorker( currReduceWorker );
	
	// ... //
	
	setReaderInfoForReduce( currReduceWorker, readerHost, readerPort, resultLocation );
	
	// ... //
	
	sendMessage( currReduceWorker->connection, currReduceWorker->reduce_info->readerHost );
	
	memset( serverMessage, 0, 1024 );
	recv( currReduceWorker->connection, serverMessage, 1024, 0 );
	
	sendMessage( currReduceWorker->connection, currReduceWorker->reduce_info->readerPort );
	
	recv( currReduceWorker->connection, serverMessage, 1024, 0 );
	
	sendMessage( currReduceWorker->connection, currReduceWorker->reduce_info->resultLocation );
	
	// ... //
	
	// I'm pretty sure that I wrote this piece of code yesterday! Where did it go??
	
	memset( serverMessage, 0, 1024 );
	recv( currReduceWorker->connection, serverMessage, 1024, 0 );
	
	sendMessage( currReduceWorker->connection, REDUCE_FUNCTION_HOST );
	
	memset( serverMessage, 0, 1024 );
	recv( currReduceWorker->connection, serverMessage, 1024, 0 );
	
	sendMessage( currReduceWorker->connection, REDUCE_FUNCTION_PORT );
	
	// ... //
	
	listenToWorker( currReduceWorker, currSearch, NULL );
}

void assignReduceWorker( worker_t *currWorker, search_node_t *currSearch )
{
	char resultLocation[ 1024 ];
	
	// ... //
	
	// Get the location of the result from Map Worker
	
	sendMessage( currWorker->connection, "LOCATION\n" );
	
	memset( resultLocation, 0, 1024 );
	recv( currWorker->connection, resultLocation, 1024, 0 );
	
	// ... //
	
	close( currWorker->connection );
	
	currWorker->state = STATE_COMPLETED;
	
	// ... //
	
	// Select Reduce Worker
	
	worker_t *currReduceWorker = NULL;
	
	while ( 1 )
	{
		currReduceWorker = getIdleReduceWorker();
		
		if ( currReduceWorker != NULL )
			break;
	}
	
	////printf( "Reduce Worker has been selected %s:%d\n", currReduceWorker->host, currReduceWorker->port );
	
	// ... //
	
	char readerPortBuffer[ 20 ];
	
	sprintf( readerPortBuffer, "%d", currWorker->readerPort );

	// ... //
	
	connectToReduceWorker( currReduceWorker, currWorker->host, readerPortBuffer, resultLocation, currSearch );
}

void *schedule( void *params )
{
	printf( "    -> Schedule Called\n\t" );
	
	scheduler_parameters_t *currParams = ( scheduler_parameters_t * ) params;
	
	printf( "    1\n" );
	
	// ... //
	
	worker_t *currWorker = currParams->currWorker; printf( "    2\n" );
	search_node_t *currSearch = currParams->currSearch; printf( "    3\n" );
	
	// ... //
	
	printf( "Current Worker for thread (%d) %s:%d\n", currParams->fileId - 1, currWorker->host, currWorker->port);
	
	//if ( currWorker->connection_state != CONNECTED )
	currWorker = connectToWorker( currWorker );
	
	sendItemsToMapWorker( currWorker, currSearch, currParams->fileId );
	
	// ... //
	
	currWorker->state = STATE_WORKING;
	
	// ... //
	
	int searchState = listenToWorker( currWorker, currSearch, currParams );
	
	// Worker failure. But backup found. just kill this thread.
	if ( searchState == 0 )
	{
		printf( "Worker failure. But backup found. just kill this thread.\n" );
		return NULL;
	}
	
	// ... //
	
	// The search process is done. Get the information which required by reduce workers then
	// send them to the candidate reduce worker to do its job.
	assignReduceWorker( currWorker, currSearch );
	
	// ... //
	
	//currWorker->state = STATE_IDLE;
	close( currWorker->connection );
}

worker_t *getIdleMapWorker()
{
	worker_t *currWorker = mapWorkers->head;
	
	while ( currWorker != NULL )
	{
		if ( currWorker->state == STATE_IDLE || currWorker->state == STATE_COMPLETED )
		{
			currWorker->state = STATE_WORKING;
			
			return currWorker;
		}
		
		currWorker = currWorker->next;
	}
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
		
		if ( messageSize <= 0 )
			continue;
		
		clearCRLF( clientMessage );
		
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
	}
	
	searchItems = originalSearchItems;
	
	*numberOfItems = currItemIdx;
	
	return searchItems;
}

void *createSearchProcess( void *params )
{
	scheduler_parameters_t *currParams = ( scheduler_parameters_t * ) params;
	
	worker_t *currWorker = currParams->currWorker;
	search_node_t *currSearch = currParams->currSearch;
	
	printf( "====> Allocated Map Worker %s:%d by %d\n", currWorker->host, currWorker->port, currParams->fileId );
	
	// ... //
	
	pid_t child = fork();
	
	if ( child == 0 )
	{
		schedule( params );
		
		printf( "List of Results are Here: \n" );
	
		result_node_t *currResult = currSearch->resultList->head;
	
		while ( currResult != NULL )
		{
			char resultMessage[ 800 ];
		
			printf( "%s:%d @ %s\n", currResult->resultWorker->host, currResult->resultWorker->readerPort, currResult->resultLocation );
		
			sprintf( resultMessage, "%s:%d@%s\n", currResult->resultWorker->host, currResult->resultWorker->readerPort, currResult->resultLocation );
		
			////////sendMessage( currParams->client, resultMessage );
		
			currResult = currResult->next;
		}
		
		_exit( EXIT_SUCCESS );
	}
	else
	{
		int status;
		//wait( &status );
		
		waitpid( child, &status, WUNTRACED );
		
		currWorker->state = STATE_IDLE;
		
		printf( "====> Freed Map Worker %s:%d by %d. And Exit Status = %d\n", currWorker->host, currWorker->port, currParams->fileId, status );
	}
}

void *recvRequest( void *recvConnection )
{
	printf( "Master: New Connection Recieved\n" );
	
	int connection = ( intptr_t ) recvConnection;
	int itemsNumber;
	pthread_t workersThreads[ NUMBER_OF_AVAILABLE_FILES ];
	
	char **searchItems = recieveItems( connection, &itemsNumber ); //getItems( &itemsNumber );
	
	// ... //
	
	search_node_t *currSearch = addSearch( itemsNumber, searchItems );
	
	// ... //
	
	worker_t *currWorker = NULL;
	int currWorkerIdx = 0;
	
	for ( ; currWorkerIdx < NUMBER_OF_AVAILABLE_FILES; currWorkerIdx++ )
	{
		printf( "\t--> currWorkerIdx = %d\n", currWorkerIdx );
		while ( 1 )
		{
			currWorker = getIdleMapWorker();
		
			if ( currWorker != NULL )
				break;
		}
		
		printf( "\tWorker Selected --> currWorkerIdx = %d, %s:%d - %d\n", currWorkerIdx, currWorker->host, currWorker->port, currWorker->state );
	
		scheduler_parameters_t *currScheduleParameters = malloc( sizeof( scheduler_parameters_t ) );
	
		currScheduleParameters->currSearch = currSearch;
		currScheduleParameters->currWorker = currWorker;
		currScheduleParameters->fileId = currWorkerIdx + 1;
		currScheduleParameters->client = connection;
		
		// ... //
		
		pthread_create( &workersThreads[ currWorkerIdx ], NULL, createSearchProcess, ( void * ) currScheduleParameters );
	}
	
	// ... //
	
	for ( int m = 0; m < currWorkerIdx; m++ )
		pthread_join( workersThreads[ m ], NULL );
	
	printf( "Process Done\n" );
	
	sendMessage( connection, "DONE" );
	
	// ... //
	
	close( connection );
	
	printf( "Connection Closed\n" );
	
	free( searchItems );
}

void newConnectionHandler( int connection )
{
	pid_t child = fork();
	
	if ( child == 0 )
	{
		recvRequest( ( void * ) ( intptr_t ) connection );
		_exit( EXIT_SUCCESS );
	}
}

int main()
{
	initLists();
	initSearchList();
	
	createServer( newConnectionHandler, 3539 );
	
	return 0;
}
