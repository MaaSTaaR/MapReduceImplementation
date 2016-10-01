#define NUMBER_OF_OCC_BEFORE_CACHE 1000

typedef struct ref_node
{
	char *word;
	int refCount;
	struct ref_node *next;
	char **cache;
	int numberOfCachedItems;
	int listId;
} ref_node_t;

typedef struct ref_list
{
	ref_node_t *head, *tail;
} ref_list_t;

ref_list_t *cacheList;

void initCache();
ref_node_t *newReference( char *word, int listId );
void incrementCounter( ref_node_t *currWord );
ref_node_t *findWordCache( char *word, int listId );
