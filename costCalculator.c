#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <stddef.h>
#include <sys/stat.h>
#include <pthread.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "queue.h"

/* --- 
TYPE & STRUCT DEFINITION
--- */

struct S_DATA_MACHINE{
    /*
    Contains the data for each machine
    */
   int machine_ID;          // Machine ID
   int machine_Type;        // Machine Type
   int machine_Time;        // Machine Time
};

typedef struct S_DATA_MACHINE DATA_MACHINE;

struct fragment{
    /*
    Contains the positions each producer has to execute
    */
    int begin_position;
    int end_position;
};


/* ---
GLOBAL DEFINITIONS
--- */

#define NUM_CONSUMERS            1                    // Number of consumers
#define MAX_BUFFER               255                  // Generic buffer for operations
#define SIZE_DATA_MACHINE        sizeof(DATA_MACHINE) // Size of a DATA_MACHINE structure


/* ---
Global Variables
--- */

const char *fNameData;
int num_Operations = -1;
int max_Operations = 0;
int total;
DATA_MACHINE* array_Operations;
pthread_mutex_t mutex;
pthread_cond_t cond_full;
pthread_cond_t cond_empty;
struct queue *circular_queue;


int load_fData(void){
    /*
    Loads the file into the struct DATA_MACHINE*

    @returns: 0 if no error, -1 on error
    */

    FILE* fpData;
    int   nBytes;
    char  buff[MAX_BUFFER];
  
    fpData = fopen(fNameData, "r");

    if (fpData != NULL){ // open OK, load file

        while (!feof(fpData)){ // read file

            if (num_Operations == -1){ 
                /* First line, number of operations to be done */
                fscanf(fpData, "%d", &max_Operations); // save on max_Operations
            }	 
            else{
                if (num_Operations == 0){
                    /* no operation saved, only max */
                    array_Operations = (DATA_MACHINE*) malloc(SIZE_DATA_MACHINE); // allocate space for struct
                }
                else{
                    array_Operations = (DATA_MACHINE*) realloc(array_Operations, SIZE_DATA_MACHINE*(num_Operations + 1));
                    // allocate space for next item
                }  
                    
                /* fill structure */
                fscanf(fpData, "%d", &array_Operations[num_Operations].machine_ID);
                fscanf(fpData, "%d", &array_Operations[num_Operations].machine_Type);
                fscanf(fpData, "%d", &array_Operations[num_Operations].machine_Time);
                   
            }  
            num_Operations++; 
        }

        fclose(fpData); // close file
        num_Operations--;

        if (num_Operations < max_Operations){
            return -1;
        }

        return 0;	   	
    }
}

int check_Params(const char* sProducers, const char* sbSize){
    /*
    Checks that all parameters for the main process are correct

    @returns: 0 if no error, -1 on error
    */
    int bSize = 0;
    int check_producers = atoi(sProducers);

    if (check_producers <= 0){
        return -1;
    }
    else{
        bSize = atoi(sbSize);
        if (bSize <= 0){
            return -1;
        }
   	}
    return 0;
}


void *consumer(){
    /*
    Consumer function.
    Takes all the elements from the queue and calculates the cost.

    @return int accum: accumulated cost
    */
    for (int i = 0; i < max_Operations; i++){

        pthread_mutex_lock(&mutex); // lock the critical section with mutex

        while(queue_empty(circular_queue)){
            /* wait for queue to fill up */
            pthread_cond_wait(&cond_empty, &mutex);
        }

        struct element *current = queue_get(circular_queue); // get element from the circular queue 
        int cost;
        /* cost calculator */
        switch (current->type){
        case 1: /* common node */
            cost = 1; 
            break;
        case 2: /* computer node */
            cost = 3;
            break;
        case 3: /* supercomputer node */
            cost = 10;
            break;
        default:
            perror("Wrong type of an element");
            break;
        }

        /* update the total */
        total += cost * current->time;

        /* unlock the critical section */
        pthread_cond_signal(&cond_full);
        pthread_mutex_unlock(&mutex);
    }

    pthread_exit(0); // terminate thread
}


void *producer(struct fragment *partition){
    /*
    Producer function.
    Inserts the data into the queue
    */
    struct fragment *producer_fragment = partition;
  
    for(int i = producer_fragment->begin_position; i < producer_fragment->end_position; i++){

        /* locking the critical section with the mutex */
        pthread_mutex_lock(&mutex);

        while(queue_full(circular_queue)){
            /* while the circular queue is full there cannot be any insertions of the producers*/
            pthread_cond_wait(&cond_full, &mutex);
        }

        /* Insert element into queue */
        DATA_MACHINE current = array_Operations[i]; // extract element
        struct element new_element; // element to be inserted on queue
        new_element.type = current.machine_Type; // get the type of machine
        new_element.time = current.machine_Time; // get the machine time
        queue_put(circular_queue, &new_element); //enqueue the element in the circular buffer
        
        /* unlock the critical section */
        pthread_cond_signal(&cond_empty);
        pthread_mutex_unlock(&mutex);
    }

    pthread_exit(0); // end the thread
}


int main (int argc, const char * argv[]){
	/*
    Entry point
    @param int argc: argument counter
    @param char *argv: argument vector
    @return 0
    */

    /* Init GLOBAL variables */ 
    fNameData = argv[1];
    num_Operations = -1;
    array_Operations = NULL;

    /* check parameters */
	if (argc != 4){
      	perror("Wrong number of parameters");
        return -1;
    }
    if (check_Params(argv[2], argv[3]) == -1){ 
    	perror ("Wrong parameters introduced");
    	return -1;
    }

    /* load file */
    if (load_fData() == -1){
       	perror("Error loading data from file");
       	return -1;
    }

    int number_Producers = atoi(argv[2]);
    int buff_size = atoi(argv[3]);

    
    /* Initialize queue */
    circular_queue = queue_init(buff_size); 

    /* Initialize mutex and conditions */
   	if (pthread_mutex_init(&mutex, NULL) < 0){
        perror("Error initializing mutex");
        exit(-1);
    }

    if (pthread_cond_init(&cond_full, NULL) < 0){
        perror("Error creating condition");
        exit(-1);
    }
  
    if (pthread_cond_init(&cond_empty, NULL) < 0){
        perror("Error creating condition");
        exit(-1);
    }
    
    /* Creation of thread consumer */
    pthread_t consumer_thread;

    if (pthread_create(&consumer_thread, NULL, (void *)consumer, NULL) < 0){
        perror("Error when creating the thread");
        exit(-1);
    }
    /* Number of operations each producer will do */
    int operations_producer = max_Operations / number_Producers; 

    /* Calculate leftovers */
    int leftovers = max_Operations % number_Producers;          

    if(leftovers){
        /* create an extra thread for leftovers */
        struct fragment fragments[number_Producers + 1]; // parameter for the treads
        pthread_t producers[number_Producers + 1];

        /* assigning fragments & thread creation */
        for(int producer_id = 0; producer_id < number_Producers; producer_id++){

            fragments[producer_id].begin_position = producer_id * operations_producer;
            fragments[producer_id].end_position = (producer_id + 1) * operations_producer;

            if (pthread_create(&producers[producer_id], NULL, (void *)producer, &fragments[producer_id]) < 0){
                perror("Error when creating the thread");
                exit(-1);
            }
        }

        /* assign leftover machines */
        fragments[number_Producers].begin_position = number_Producers * operations_producer;
        fragments[number_Producers].end_position = max_Operations;
        if (pthread_create(&producers[number_Producers], NULL, (void *)producer, &fragments[number_Producers]) < 0){
            perror("Error when creating the thread");
            exit(-1);
        }
        for (int i = 0; i < number_Producers + 1; i++){
            if (pthread_join(producers[i], NULL) < 0){
                perror("Error when waiting thread");
                exit(-1);
            }
        }
    }

    else{
        /* If there are not leftovers */
        struct fragment fragments[number_Producers];
        pthread_t producers[number_Producers]; // as many threads as producers

        /* assigning fragments & thread creation */
        for(int producer_id = 0; producer_id < number_Producers; producer_id ++){

            fragments[producer_id].begin_position = producer_id * operations_producer;
            fragments[producer_id].end_position = (producer_id + 1) * operations_producer;

            if (pthread_create(&producers[producer_id], NULL, (void *)producer, &fragments[producer_id]) < 0){
                perror("Error when creating the thread");
                exit(-1);
            }
        }
        /* Thread join */
        for (int i = 0; i < number_Producers; i++){
            if (pthread_join(producers[i], NULL) < 0){
                perror("Error when waiting thread");
                exit(-1);
            }
        }
    }


    if (pthread_join(consumer_thread, NULL) < 0){
        perror("Error when waiting the thread");
        exit(-1);
    }
    
    /* Print total */
    printf("Total: %i €.\n", total);


    /* Destroy circular buffer and mutex and coditions */
    queue_destroy(circular_queue);
    if (pthread_mutex_destroy(&mutex) < 0){
        perror("Error when destroying mutex");
        exit(-1);
    }

    if (pthread_cond_destroy(&cond_full) < 0){
        perror("Error when destroying condition");
        exit(-1);
    }

    if (pthread_cond_destroy(&cond_empty) < 0){
        perror("Error when destroying condition");
        exit(-1);
    }


	return 0;
}
     


