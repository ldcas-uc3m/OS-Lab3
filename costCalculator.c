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
GLOBAL DEFINITIONS
--- */

#define NUM_CONSUMERS            1    // Number of consumers

#define RESULT_OK                0    // General Ok return of funtcion
#define RESULT_ER                1    // General ERROR return of function
#define RESULT_ER_FILE           2    // General ERROR of file return of function
#define RESULT_ER_NUM_OPERATIONS 3
#define RESULT_ER_PRODUCERS      4
#define RESULT_ER_BSIZE          5

#define MAX_BUFFER               255  // Generic buffer for operations

/* --- 
TYPE & STRUCT DEFINITION
--- */

struct S_DATA_MACHINE{
   int machine_ID;          // Machine ID
   int machine_Type;        // Machine Type
   int machine_Time;        // Machine Time
};

typedef struct S_DATA_MACHINE DATA_MACHINE;

#define SIZE_DATA_MACHINE   sizeof(DATA_MACHINE)


/* ---
Global Variables
--- */

const char *fNameData;
int bSize;
int producers;
int num_Operations = -1;
int total;
DATA_MACHINE* array_Operations;
pthread_mutex_t mutex;
pthread_cond_t cond_full;
pthread_cond_t cond_empty;
struct queue *queue;


int load_fData(void){
    /*
    Loads the file into the struct DATA_MACHINE*

    @returns: 0 if no error, -1 on error
    */
    int   result;
    FILE* fpData;
    int   max_Operations;
    int   nBytes;
    char  buff[MAX_BUFFER];
  

    result = RESULT_ER_FILE;
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
    producers = atoi(sProducers);

    if (producers <= 0){
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


int consumer(){
    /*
    Consumer function.
    Takes all the elements from the queue and calculates the cost.

    @param struct queue *q: circular buffer queue
    @return int accum: accumulated cost
    */
    int accum = 0; // accumulator
    while (!queue_empty(queue)){
        struct element *current = queue_get(queue); // get elemet

        /* cost calculator */
        switch (current->type){
        case 1: /* common node */
            int cost = 1; // ($/min)
            break;
        case 2: /* computer node */
            int cost = 3;
            break;
        case 3: /* supercomputer node */
            int cost = 10;
            break;
        default:
            perror("Wrong type of an element");
            break;
        }
        accum += cost * current->time;
    }
    return accum;
}


void producer(int num_execution){

    return;
}


/*
void test_Load_Array(void){
  int i;
  
    for (i=0; i<num_Operations; i++){
        printf("ID:%d, TYPE:%d, TIME: %d\n",array_Operations[i].machine_ID,array_Operations[i].machine_Type,array_Operations[i].machine_Time);
	}
    printf("num_Operations:%d\n", i);
}
*/


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
    producers = 0;
    bSize = 0;

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

    int num_Producers = atoi(argv[2]);
    int buff_size = atoi(argv[3]);

    /* ---
    THREAD CREATION
    --- */

    pthread_t producers[num_Producers]; // as many threads as producers
    pthread_t consumer;

    int operations_producer = (buff_size/num_Producers); // Number of operations each producer will do

    queue = queue_init(buff_size);

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
	
    /* output */
	printf("%i", total);
	return 0;
}
     


