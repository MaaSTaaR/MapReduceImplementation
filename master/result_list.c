#include <stdlib.h>
#include <string.h>

#include "result_list.h"

result_list_t *initResultList()
{
	result_list_t *currList = malloc( sizeof( result_list_t ) );
	
	currList->head = NULL;
	currList->tail = NULL;
	
	return currList;
}

void addNewResult( result_list_t *currList, worker_t *resultWorker, char *resultLocation )
{
	result_node_t *newResult = malloc( sizeof( result_node_t ) );
	
	newResult->resultWorker = resultWorker;
	strcpy( newResult->resultLocation, resultLocation );
	
	if ( currList->head == NULL )
	{
		currList->head = newResult;
		currList->tail = newResult;
	}
	else
	{
		currList->tail->next = newResult;
		currList->tail = newResult;
	}
}
