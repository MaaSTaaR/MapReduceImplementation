#include <stdlib.h>

#include "search_list.h"

void initSearchList()
{
	searchList = malloc( sizeof( search_list_t ) );
	
	searchList->head = NULL;
	searchList->tail = NULL;
}

search_node_t *addSearch( int itemsNumber, char **searchItems )
{
	search_node_t *newSearch = malloc( sizeof( search_node_t ) );
	
	newSearch->itemsNumber = itemsNumber;
	newSearch->items = searchItems;
	newSearch->resultList = malloc( sizeof( result_list_t ) );
	
	if ( searchList->head == NULL )
	{
		searchList->head = newSearch;
		searchList->tail = newSearch;
	}
	else
	{
		searchList->tail->next = newSearch;
		searchList->tail = newSearch;
	}
	
	return newSearch;
}
