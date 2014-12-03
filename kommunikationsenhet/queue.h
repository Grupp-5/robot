
/*	queue.h

	Header file for queue implementation

	by: Steven Skiena
*/

/*
Copyright 2003 by Steven S. Skiena; all rights reserved.

Permission is granted for use in non-commerical applications
provided this copyright notice remains intact and unchanged.

This program appears in my book:

"Programming Challenges: The Programming Contest Training Manual"
by Steven Skiena and Miguel Revilla, Springer-Verlag, New York 2003.

See our website www.programming-challenges.com for additional information.

This book can be ordered from Amazon.com at

http://www.amazon.com/exec/obidos/ASIN/0387001638/thealgorithmrepo/

*/

#include <stdint.h>
#include <stdbool.h>
#include <common.h>

#define QUEUESIZE       20

typedef struct {
	Bus_data q[QUEUESIZE+1]; /* body of queue */
	uint8_t first;           /* position of first element */
	uint8_t last;            /* position of last element */
	uint8_t count;           /* number of queue elements */
} queue;


void init_queue(queue *q);
void enqueue(queue *q, Bus_data x);
Bus_data dequeue(queue *q);
bool empty(queue *q);
