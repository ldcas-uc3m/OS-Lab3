#ifndef HEADER_FILE
#define HEADER_FILE


struct element {
  int type; //Machine type
  int time; //Using time
};

typedef struct queue {
	// Define the struct yourself
	/* The queue is an array of pointers to the structure elements */
	struct element *array;

	/* The current size the queue has at each moment */
	int size;

	/* Maximum size the queue can have */
	int max_size;

	/* Integer specifying the position of the array where the head is */
	int head;

	/* Integer specifying the position of the array where the tail is */
	int tail;
}queue;

queue* queue_init (int size);
int queue_destroy (queue *q);
int queue_put (queue *q, struct element* elem);
struct element * queue_get(queue *q);
int queue_empty (queue *q);
int queue_full(queue *q);

#endif
