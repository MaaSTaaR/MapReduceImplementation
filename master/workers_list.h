#define STATE_IDLE 0
#define STATE_WORKING 1
#define STATE_COMPLETED 2
#define STATE_DEAD 3
#define CONNECTION_NOT_ESTABLISHED 4
#define CONNECTED 5
#define DISCONNECTED 6
#define WORKER_TYPE_MAP 7
#define WORKER_TYPE_REDUCE 8

typedef struct reduce_info
{
	char *readerHost, *readerPort, *resultLocation;
} reduce_info_t;

typedef struct worker
{
	char host[ 1000 ];
	int port;
	int readerPort;
	int state;
	int setId;
	int connection;
	int connection_state;
	int type;
	reduce_info_t *reduce_info;
	struct worker *next;
	struct list *backup;
} worker_t;


typedef struct list
{
	worker_t *head;
	worker_t *tail;
	int numberOfWorkers;
} list_t;

list_t *mapWorkers, *reduceWorkers;

void initLists();
worker_t *addWorker( list_t *currList, char *host, int port, int readerPort, int type );
void foreach( list_t *currList, void ( *callback )( worker_t *currNode ) );
worker_t *addBackup( worker_t *currWorker, char *host, int port, int readerPort );
void setReaderInfoForReduce( worker_t *reduceWorker, char *readerHost, char *readerPort, char *resultLocation );
