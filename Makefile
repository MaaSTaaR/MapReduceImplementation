COMPILER = gcc
MAP_WORKER_FILES = workers/map/map.c workers/map/cache.c core/server/server.c core/search/search.c
REDUCE_WORKER_FILES = workers/reduce/reduce.c core/server/server.c
READER_FILES = workers/reader.c core/server/server.c
MASTER_FILES = master/master.c master/workers_list.c master/search_list.c master/result_list.c core/server/server.c
MAP_FUNCTION_FILES = master/functions/search.c core/server/server.c core/search/search.c
REDUCE_FUNCTION_FILES = master/functions/identity.c core/server/server.c
CLIENT_FILES = client.c core/server/server.c

build: $(MAP_WORKER_FILES) $(REDUCE_WORKER_FILES) $(READER_FILES) $(MASTER_FILES) $(MAP_FUNCTION_FILES) $(REDUCE_FUNCTION_FILES)
	$(COMPILER) $(MAP_WORKER_FILES) -o bin/map_worker.o -lpthread -g
	$(COMPILER) $(REDUCE_WORKER_FILES) -o bin/reduce_worker.o -lpthread -g
	$(COMPILER) $(READER_FILES) -o bin/reader.o -lpthread
	$(COMPILER) $(MASTER_FILES) -o bin/master.o -lpthread -g
	$(COMPILER) $(MAP_FUNCTION_FILES) -o bin/map_function_search.o -lpthread -g
	$(COMPILER) $(REDUCE_FUNCTION_FILES) -o bin/reduce_function_identity.o -lpthread -g
	$(COMPILER) $(CLIENT_FILES) -o bin/client.o -lpthread

clean:
	rm -f bin/*.o
