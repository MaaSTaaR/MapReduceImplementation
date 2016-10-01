#include "result_list.h"

typedef struct search_node
{
	int itemsNumber;
	char **items;
	result_list_t *resultList;
	struct search_node *next;
} search_node_t;


typedef struct search_list
{
	search_node_t *head;
	search_node_t *tail;
} search_list_t;

search_list_t *searchList;

void initSearchList();
search_node_t *addSearch( int itemsNumber, char **searchItems );
