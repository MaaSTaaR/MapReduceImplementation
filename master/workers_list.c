#include <stdlib.h>
#include <string.h>

#include "workers_list.h"

int numberOfSets = 0;

list_t *initList()
{
	list_t *currList = malloc( sizeof( list_t ) );
	
	currList->head = NULL;
	currList->tail = NULL;
	currList->numberOfWorkers = 0;
	
	return currList;
}

void addWorkers()
{
	worker_t *currWorker;
	
	// ... //
	
	// Map Workers
	
	currWorker = addWorker( mapWorkers, "127.0.0.1", 5390, 5400, WORKER_TYPE_MAP );
	
	//addBackup( currWorker, "127.0.0.1", 5394, 5404 );
	
	// ... //
	
	currWorker = addWorker( mapWorkers, "127.0.0.1", 5391, 5401, WORKER_TYPE_MAP );
	currWorker = addWorker( mapWorkers, "127.0.0.1", 5392, 5402, WORKER_TYPE_MAP );
	//currWorker = addWorker( mapWorkers, "127.0.0.1", 5393, 5403, WORKER_TYPE_MAP );
	//currWorker = addWorker( mapWorkers, "127.0.0.1", 5392, 5402 );
	//currWorker = addWorker( mapWorkers, "127.0.0.1", 5393, 5403 );
	
	// ... //
	
	// Reduce Workers
	
	addWorker( reduceWorkers, "127.0.0.1", 5500, -1, WORKER_TYPE_REDUCE );
	addWorker( reduceWorkers, "127.0.0.1", 5501, -1, WORKER_TYPE_REDUCE );
	addWorker( reduceWorkers, "127.0.0.1", 5502, -1, WORKER_TYPE_REDUCE );
	addWorker( reduceWorkers, "127.0.0.1", 5503, -1, WORKER_TYPE_REDUCE );
}

void initLists()
{
	mapWorkers = initList();
	reduceWorkers = initList();
	
	addWorkers();
}

worker_t *addWorker( list_t *currList, char *host, int port, int readerPort, int type )
{
	worker_t *newWorker = malloc( sizeof( worker_t ) );

	strcpy( newWorker->host, host );
	
	newWorker->port = port;
	newWorker->backup = malloc( sizeof( list_t ) );
	newWorker->state = STATE_IDLE;
	newWorker->connection_state = CONNECTION_NOT_ESTABLISHED;
	newWorker->readerPort = readerPort;
	newWorker->type = type;
	
	//if ( type == WORKER_TYPE_REDUCE )
		newWorker->reduce_info = malloc( sizeof( reduce_info_t ) );
	
	numberOfSets++;
	
	newWorker->setId = numberOfSets;
	
	if ( currList->head == NULL )
	{
		currList->head = newWorker;
		currList->tail = newWorker;
	}
	else
	{
		currList->tail->next = newWorker;
		currList->tail = newWorker;
	}
	
	currList->numberOfWorkers++;
	
	return newWorker;
}

void foreach( list_t *currList, void ( *callback )( worker_t *currNode ) )
{
	worker_t *currNode = currList->head;
	
	while ( currNode != NULL )
	{
		callback( currNode );
		
		currNode = currNode->next;
	}
}

worker_t *addBackup( worker_t *currWorker, char *host, int port, int readerPort )
{
	return addWorker( currWorker->backup, host, port, readerPort, currWorker->type );
}

void setReaderInfoForReduce( worker_t *reduceWorker, char *readerHost, char *readerPort, char *resultLocation )
{
	reduceWorker->reduce_info->readerHost = malloc( strlen( readerHost ) );
	reduceWorker->reduce_info->readerPort = malloc( strlen( readerPort ) );
	reduceWorker->reduce_info->resultLocation = malloc( strlen( resultLocation ) );
	
	strcpy( reduceWorker->reduce_info->readerHost, readerHost );
	strcpy( reduceWorker->reduce_info->readerPort, readerPort );
	strcpy( reduceWorker->reduce_info->resultLocation, resultLocation );
}
