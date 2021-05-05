


#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include "queue.h"



//To create a queue
queue* queue_init(int size){
	/* Reserve memory for the queue */
    queue * q = (queue *)malloc(sizeof(queue));

    /* Reserve memory for the array, we have to reserve maximum number of elements 
    the queue can have times the size of each element that is an structure*/
    q->array = malloc(size * sizeof(struct element));

    /* As the queue is initially empty the head and the tail are set to position 0*/
    q->head = 0;
    q->tail = 0;

    /* Initial size is 0 */
    q->size = 0;

    /* The maximum size is given as parameter*/
    q->max_size = size;   	
    return q;
}


// To Enqueue an element
int queue_put(queue *q, struct element* x) {
	/* We should enqueue only when the queue is not full*/
	if(queue_full(q) != 1){
		/* The array works as a circular array, we need to update the head according to the maximum size */
		q->head = (q->head + 1) % q->max_size;

		/* Assign the element to array at position of the head */
		q->array[q->head] = *x;

		/* Increment the size */
		q->size = q->size + 1;
        return 1
	}
    return 0;
}


// To Dequeue an element.
struct element* queue_get(queue *q) {
	/* Initialize the element to be returned */
    struct element* element;

    /* We can only dequeue if the queue is not empty */
    if (queue_empty(q) != 1){

    	/* Get the element in the tail */
    	element = &q->array[q->tail];

    	/* Update the tail, as the array is a circular array we use the remainder and the maximum size*/
    	q->tail = (q->tail + 1) % q->max_size;

    	/* Decrement the size */
    	q->size = q->size - 1;
        return element;
    }
    return 0
}


//To check queue state
int queue_empty(queue *q){
	/* Retrun 1 if the queue is empty, otherwise return 0 */
	if (q->size == 0){
		return 1;
	}
    return 0;
}

int queue_full(queue *q){
	/* Retrun 1 if the queue is full, otherwise return 0 */
    if (q->size == q->max_size){
		return 1;
	}
    return 0;
}

//To destroy the queue and free the resources
int queue_destroy(queue *q){
	/* Free the queue */
	free(q);
    return 0;
}
