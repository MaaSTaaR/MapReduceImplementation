#include "workers_list.h"

typedef struct result_node
{
	worker_t *resultWorker;
	char resultLocation[ 200 ];
	struct result_node *next;
} result_node_t;

typedef struct result_list
{
	result_node_t *head, *tail;
} result_list_t;

result_list_t *initResultList();
void addNewResult( result_list_t *currList, worker_t *resultWorker, char *resultLocation );
