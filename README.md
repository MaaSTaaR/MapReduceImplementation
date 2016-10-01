Simple MapReduce Implementation
===============================

A simple MapReduce implementation in C based on Google's paper "[MapReduce: Simplified Data Processing on Large Clusters](http://jayurbain.com/msoe/cs4230/Readings/MapReduce%20-%20Simplified%20Data%20Processing%20on%20Large%20Clusters.pdf)" under the supervision of Prof. [Hussain Almohri](http://www.halmohri.com).

In this implementation Map & Reduce functions are simple TCP/IP server that receive a line from the worker (map or reduce) process it and send it back to the worker. For now search and identity functions are implemented. The path of input data can be found in the file "workers/map/map.c".

By using "make" command the binaries will be generated in "bin" directory. They will be:
* client.o: A test program that request from MapReduce to process a request.
* map_function_search.o: Map function implementation to search on the files.
* reduce_function_identity.o: Reduce function implementation. The identity function.
* map_worker.o: Map worker. Must be run on Map nodes.
* reduce_worker.o: Reduce worker. Must be run on Reduce nodes.
* reader.o: The reader which is used by reduce workers to read map output.
* master.o: The master of the nodes.

The cluster workers are defined on the file "master/workers_list.c".

License: GNU GPL.
