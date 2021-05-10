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
   int machine_ID;          // Machine ID
   int machine_Type;        // Machine Type
   int machine_Time;        // Machine Time
};

typedef struct S_DATA_MACHINE DATA_MACHINE;

struct fragment {
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
int bSize;
int check_producers;
int num_Operations = -1;
int operations_producer = 0;
int buff_size = 0;
int max_Operations = 0;
int total;
DATA_MACHINE* array_Operations;
pthread_mutex_t mutex;
pthread_cond_t cond_full;
pthread_cond_t cond_empty;
struct queue *buff_q;


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

int check_Params(const char* sProducers,const char* sbSize){
    /*
    Checks that all parameters for the main process are correct

    @returns: 0 if no error, -1 on error
    */
    check_producers = atoi(sProducers);

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
        int accum = 0; // accumulator
        pthread_mutex_lock(&mutex);

        while(buff_q->size == 0){
            pthread_cond_wait(&cond_empty, &mutex);
        }

        struct element *current = queue_get(buff_q); // get elemet
        int cost;
        /* cost calculator */
        switch (current->type){
        case 1: /* common node */
            cost = 1; // ($/min)
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

        accum += cost * current->time;
        total = total + accum;

        pthread_cond_signal(&cond_full);
        pthread_mutex_unlock(&mutex);
    }

    pthread_exit(0);
}


void *producer(struct fragment *partition){
    /*
    Producer function.
    Inserts the data into the queue
    */
    printf("oppp %d\n", operations_producer);
    struct fragment *producer_fragment = partition;
  
    for(int i = producer_fragment->begin_position; i < producer_fragment->end_position; i++){

        pthread_mutex_lock(&mutex);

        while(buff_q->size == buff_size){
            pthread_cond_wait(&cond_full, &mutex);
        }



        // ------------------------------------------------
        // EN ALGUNOS CASOS   (*producer_number * operations_producer) + i GENERA UN VALOR ERRONEO CON EL QUE NO SE PUEDE INDEXAR EN array_Operations
        // ------------------------------------------------
        long int index;
        //printf("*producer_number: %d\n",*producer_number);
        //printf("operations_producers: %d\n",operations_producer);
        //printf("i: %d\n",i);
        index = (*producer_number * operations_producer) + i;
        

        if ( index>0 && index<num_Operations) {

        DATA_MACHINE current = array_Operations[i]; // extract element
        //struct element *new_element; // element to be inserted on queue

        //new_element->type = current.machine_Type; // insert type
        //new_element->time = current.machine_Time;

        struct element new_element; // element to be ins     erted on queue

        new_element.type = current.machine_Type; // insert type
        new_element.time = current.machine_Time;
        queue_put(buff_q, &new_element);
        }

        else {
            printf("ERROR DE INDICE: %ld\n",index);
        }
            //queue_put(buff_q, new_element);
        pthread_cond_signal(&cond_empty);

        pthread_mutex_unlock(&mutex);
 
    }

    pthread_exit(0);
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
    check_producers = 0;
    bSize = 0;

    printf("Paso 1\n");
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
    printf("Paso 2\n");
    int num_Producers = atoi(argv[2]);
    buff_size = atoi(argv[3]);

    /* ---
    THREAD CREATION
    --- */

    pthread_t producers[num_Producers]; // as many threads as producers
    pthread_t consumer_t;


    operations_producer = max_Operations / num_Producers; // Number of operations each producer will do

   
    printf("Paso 3\n");
    
    buff_q = queue_init(buff_size);
    printf("Paso 4\n");

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
	
    printf("Paso 5\n");
    if (pthread_create(&consumer_t, NULL, (void *)consumer, NULL) < 0){
        perror("Error when creating the thread");
        exit(-1);
    }

    struct fragment fragments[num_Producers];
    printf("Paso 6\n");
    for (int producer_numb = 0; producer_numb < num_Producers; producer_numb++){

        fragments[producer_numb].begin_position = producer_numb * operations_producer;
        fragments[producer_numb].end_position = (producer_numb + 1) * operations_producer;

        if (pthread_create(&producers[producer_numb], NULL, (void *)producer, &fragments[producer_numb]) < 0){
            perror("Error when creating the thread");
            exit(-1);
        }
    }
    
    
    printf("Paso 7\n");
    for (int i = 0; i < num_Producers; i++){
         if (pthread_join(producers[i], NULL) < 0){
            perror("Error when waiting thread");
            exit(-1);
        }
    }

    printf("Paso 8\n");
    if (pthread_join(consumer_t, NULL) < 0){
        perror("Error when waiting the thread");
        exit(-1);
    }
    
    printf("Total: %i €.\n", total);
    printf("Paso 9\n");
    queue_destroy(buff_q);
    if (pthread_mutex_destroy(&mutex) < 0){
        perror("Error when destroying mutex");
        exit(-1);
    }

    printf("Paso 10\n");
    if (pthread_cond_destroy(&cond_full) < 0){
        perror("Error when destroying condition");
        exit(-1);
    }

    printf("Paso 11\n");
    if (pthread_cond_destroy(&cond_empty) < 0){
        perror("Error when destroying condition");
        exit(-1);
    }

    printf("Paso 12\n");

	return 0;
}
     


