#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cache.h"

void initCache()
{
	cacheList = malloc( sizeof( ref_list_t ) );
	
	cacheList->head = NULL;
	cacheList->tail = NULL;
}

ref_node_t *newReference( char *word, int listId )
{
	ref_node_t *newReference = malloc( sizeof( ref_node_t ) );
	
	newReference->word = malloc( sizeof( char ) * strlen( word ) );
	
	strcpy( newReference->word, word );
	
	newReference->refCount = 1;
	newReference->cache = NULL;
	newReference->numberOfCachedItems = 0;
	newReference->listId = listId;
	newReference->next = NULL;
	
	if ( cacheList->head == NULL )
	{
		cacheList->head = newReference;
		cacheList->tail = newReference;
	}
	else
	{
		cacheList->tail->next = newReference;
		cacheList->tail = newReference;
	}
	
	return newReference;
}

void incrementCounter( ref_node_t *currWord )
{
	currWord->refCount++;
}

ref_node_t *findWordCache( char *word, int listId )
{
	ref_node_t *currWord = cacheList->head;
	
	while ( currWord != NULL )
	{
		if ( strcmp( currWord->word, word ) == 0  && listId == currWord->listId )
			return currWord;
		
		currWord = currWord->next;
	}
	
	return NULL;
}
