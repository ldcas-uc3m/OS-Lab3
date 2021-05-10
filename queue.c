#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include "queue.h"


queue* queue_init(int size){
	/*
    Reserves memory for the queue, and initializes the queue

    @param int size: size of the queue
    */
    queue *q = (queue *)malloc(sizeof(queue)); // allocate queue

    q->array = malloc(size * sizeof(struct element)); // allocate for elements

    /* As the queue is initially empty the head and the tail are set to position 0*/
    q->head = 0;
    q->tail = 0;

    /* Initial size is 0 */
    q->size = 0;

    /* The maximum size is given as parameter*/
    q->max_size = size;   	
    return q;
}


int queue_put(queue *q, struct element *x){
    /*
    Introduces an element into the queue
    
    @param queue *q: queue
    @param struct element *x: element to be stored
    @return: 0 on exit, -1 on fail
    */
    
	if(q->size == 0){
        /* first element, head stays */
        q->array[q->head] = *x; // Assign the element to array at position of the head
        q->size = q->size + 1; // Increment the size

        return 0;
    }

	else if(queue_full(q) != 1){
        /* new element */
		// The array works as a circular array, we need to update the head according to the maximum size

		q->head = (q->head + 1) % q->max_size; // move head
		q->array[q->head] = *x; // Assign the element to array at position of the head
		q->size = q->size + 1; // Increment the size

        return 0;
	}
    return -1;
}


struct element* queue_get(queue *q){
	/*
    Dequeues the tail element
    
    @param queue *q: queue
    @return: struct element *element, -1 on fail
    */

   /* Initialize the element to be returned */
    struct element *new_element;
    /* Dequeuing when there is just one element */
    if(q->size == 1){
        /* Only one element, tail static */
        new_element = &q->array[q->tail]; // Get the element in the tail
        q->size = q->size - 1; // Decrement the size
        return new_element;
    }

    else if(queue_empty(q) != 1){
        /* dequeue */
    	new_element = &q->array[q->tail];
    	// as the array is a circular array we use the remainder and the maximum size
    	q->tail = (q->tail + 1) % q->max_size; // Update the tail
    	q->size = q->size - 1;

        return new_element;
    }
    return -1;
}


int queue_empty(queue *q){
	/* 
    Checks whether or not the queue is empty

    @return: 1 if the queue is empty, otherwise 0
    */

	if (q->size == 0){
		return 1;
	}
    return 0;
}


int queue_full(queue *q){
	/* 
    Checks whether the queue is full

    @return: 1 if the queue is full, otherwise 0
    */
    if (q->size == q->max_size){
		return 1;
	}
    return 0;
}


int queue_destroy(queue *q){
	/*
    Frees the queue
    
    @param queue *q: queue
    */
	free(q);
    return 0;
}
