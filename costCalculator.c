
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <stddef.h>
#include <sys/stat.h>
#include <pthread.h>
#include "queue.h"
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>


#define NUM_CONSUMERS 1



int main (int argc, const char * argv[] ){
    /*
    Entry point
    @param int argc: argument counter
    @param char *argv: argument vector
    @return 0
    */

    int total = 0;
    printf("Total: %i €.\n", total);

    return 0;
}

int consumer(struct queue *q){
    /*
    Consumer function.
    Takes all the elements from the queue and calculates the cost.

    @param struct queue *q: circular buffer queue
    @return int accum: accumulated cost
    */
    int accum = 0; // accumulator
    while (!queue_empty(q)){
        struct element *current = queue_get(q); // get elemet

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
