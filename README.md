# OS-Lab3
By team 89-09: Ignacio Arnaiz Tierraseca, Luis Daniel Casais Mezquida & Juan Del Pino Vega
## Lab statement
This practice allows the student to become familiar with the services for the management of concurrency provided by POSIX. <br/>
For the management of threads, the system calls ``pthread_create()``, ``pthread_join()`` and ``pthread_exit()``, and for the synchronization of these, mutex and conditional variables will be used:
* __pthread_create__: creates a new thread that executes a function that is specified as an argument in the call.
* __pthread_join__: waits for a thread that must terminate and that is indicated as an argument of the call.
* __pthread_exit__: ends the execution of the calling process.

The student must design and code, in C language and on the Linux operating system, a program
that acts as an accounting system in data center. <br/>
<br/>
The objective of this practice is to code a concurrent multi-threaded system that calculates the cost of using the machines in a processing center. Given a file with a specific format, it is necessary to calculate how much the customer should be charged according to the type of machines he wants to use, as well as the time he wants to use them.<br/>
For the realization of the functionality, it is recommended to implement two basic functions, which represent the roles of the program:
* __Producer__: It will be the function executed by the threads in charge of adding elements to
the shared circular queue.
* __Consumer__: It will be the function executed by the thread in charge of extracting elements
from the shared circular queue.

In this way, the program will have the following behavior:
1. The main thread will be in charge of:
    1. Read the input arguments.
    2.  Load the data from the file provided in memory.
    3. Make a distribution of the file load among the number of producing threads indicated.
    3. Launch the n producers and the consumer.
    4. Wait for the completion of all threads and display the total calculated cost (result of waiting for the end of the consumer thread).
2. Each producer thread must:
    1. Obtain the data extracted from the corresponding file and insert them, one by one, in the circular queue.
    2. This task must be carried out concurrently with the rest of the producers, as well asthe consumer. In no case can threads be blocked "manually" or force an order between them (for example, wait for a thread to insert all its elements, or those that fit in the queue, and give way to the consumer to extract them; make way for the next producer, etc.).
3. The consumer thread must:
    1. Obtain, concurrently, the elements inserted in the queue.
    2. Each item checked out represents a type of machine and usage time, so you will need to calculate the cost and accumulate it until all items have been processed.
    3. Once all the elements have been processed, the thread will end its execution, returning the total cost calculated to the main thread.

## Calculator of computing costs in a data center
The main program will take care of importing arguments and data from the indicated file. To do this, it must be taken into account that the execution of the program will be as follows:<br/>
``$ ./calculator <file_name> <num. Producers> <buff. Size>``<br/>
The "file name" label corresponds to the name of the file to be imported. The label “num. Producers ” is an integer that represents the number of producer threads to be generated. Finally, the “buff. Size ” is an integer that indicates the size of the circular queue (maximum number of items it can store).<br/>
On the other hand, the input file must have the following format:<br/>
``500``<br/>
``1 1 4``<br/>
``2 2 12``<br/>
``3 1 100``<br/>
``4 3 45``<br/>
``...``<br/>
The first line of the file represents the number of operations to be calculated. There may be more operations in the file, but only those indicated by this value should be processed. In no case can there be fewer operations in the file than operations indicated by the first value. The rest of the lines in the file represent an operation: ``<id> <machine type> <usage time>``. They are three integer values separated by a space and ended in a line break. The id is consecutive: it increases with each operation. The type of machine represents whether it is a common node (cost 1 € / minute), computing node (cost 3 € / minute), or a supercomputer (cost 10 €/ minute). The last value represents the usage time. With all this, the total cost that the consumer must calculate is: ∑(type*time).<br/>
The main process must load the information contained in the file into memory for later processing by the producers. For this, it is recommended to make use of the "scanf" function and the dynamic memory reservation with "malloc" (and "free" for later release). The idea is:
1. Obtain the number of operations (first value of the file).
2. Reserve memory for all those operations with malloc.
3. Store the operations in the array.
4. Distribute operations among producers:
    1. To simplify the task, it is recommended to distribute the operations, so that each producing thread knows in which position to start processing and in which position to end.
    2. In this way, each thread is aware of when it should end its execution.
    3. To do this, arguments can be passed to each producing thread at launch with "pthread_create".
2. After processing the threads, free the reserved memory with free.

__NOTE__: To store the data from the file, an array of structures can be generated. It is also recommended to use a structure for passing parameters to threads.<br/>
Below is an example of the program's output:<br/>
``$ ./calculator input_file 5 10``<br/>
``Total: 234234 €.``<br/>

## n-producers - 1-consumer
The problem to be implemented is a classic example of process synchronization: when sharing a shared queue (circular buffer), you have to control the concurrency when depositing objects in it, and when extracting them.<br/>
For the implementation of the producing threads it is recommended that the function follow the following scheme for simplicity:
1. Obtaining the indexes that must be accessed from the file data. It is recommended to pass parameters to the thread.
2. Loop from the beginning to the end of the operations to be processed:
    1. Obtain the transaction data.
    2. Create an element with the data of the operation to insert in the queue.
    3. Insert item in queue.
3. End the thread with pthread_exit.

For the implementation of the consumer thread, it is recommended to follow a similar scheme to the previous one for simplicity:
1. Remove item from queue.
2. Process the cost of the operation and accumulate it.
3. When all operations have been processed, end the thread with pthread_exit returning the total cost calculated.

__NOTE__: To control concurrency, you must use mutex and condition variables. Concurrency can be managed in the producer and consumer functions, or in the circular queue code (in queue.c). This is your design choice.

### Circular buffer queue
Communication between producers and consumers will be done through a circular queue. A circular queue must be created shared by the producers and the consumer. Since modifications are constantly going to be made on this element, concurrency control mechanisms must be implemented for light processes.<br/>
The circular queue and its functions must be implemented in a file called queue.c, and must contain, at least, the following functions:
* ``queue * queue_init(int num_elements)``: function that creates the queue and reserves the size specified as a parameter.
* ``int queue_destroy(queue *q)``: function that removes the queue and frees all allocated resources.
* ``int queue_put (queue *q, struct element * ele)``: function that inserts elements into the queue if space is available. If there is no space available, you must wait until the insertion can be performed.
* ``struct element *queue_get(queue *q)``: function that extracts elements from the queue if it is not empty. If this is empty, you must wait until an item is available.
* ``int queue_empty(queue *q)``: function that queries the state of the queue and determines if it is empty (return 1) or not (return 0).
* ``int queue_full(queue *q)``: function that queries the state of the queue and determines if it is full (return 1) or if it still has available positions (return 0).

The implementation of this queue must be done in such a way that there are no concurrency problems between the threads that are working with it. For this, the proposed mechanisms of mutex and condition variables must be used.<br/>
The object to be stored and extracted from the circular queue must correspond to a structure defined with the following fields, at least:
* ``int type``: 1 if it is a common node; 2 if it is a compute node; 3 if it is a super computer.
* ``int time``: represents the time that the machine will be used with the characteristics defined by type.

## Initial code
We provide a file “ssoo_p3_initial_code.zip”. When decompressed, a “./ssoo_p3_initial_code” directory is created when you will find the files to make the lab:
* costCalculator.c
* queue.c
* queue.h:
* Makefile

## Lab	results	verification
Along with the support code, a “checker.csv” file is attached, which is an excel document to check if the cost results obtained are correct or not. The file "file.txt" provided contains the same data as the first one, but without formulas and checks (it has the specified format to be able to process it). If the number of operations to be processed in "file.txt" is modified, the result obtained can be checked by modifying the number of operations in the document "checker.csv".
``$ ./corrector_ssoo_p3.sh ssoo_p3_100428997_100429021_100429063.zip``
