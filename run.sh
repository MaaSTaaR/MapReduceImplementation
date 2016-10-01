./bin/master.o &	PIDMASTER = $!

./bin/map_worker.o 1
./bin/map_worker.o 2 &  
./bin/map_worker.o 3 &  
./bin/map_worker.o 4 &  

./bin/map_function_search.o &

./bin/reader.o 0 map5390 &
./bin/reader.o 0 map5391 &  
./bin/reader.o 0 map5392 &  
./bin/reader.o 0 map5393 & 

./bin/reduce_worker.o 0
./bin/reduce_worker.o 1
./bin/reduce_worker.o 2
./bin/reduce_worker.o 3

./bin/reduce_function_identity.o &

wait $PIDMASTER
