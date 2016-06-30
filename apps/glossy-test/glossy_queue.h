/*
 * glossy_queue.h
 *
 *  Created on: 27-Jun-2016
 *      Author: NithinTomy
 */

#ifndef GLOSSY_QUEUE_H_
#define GLOSSY_QUEUE_H_



#include<stdio.h>
#include<stdlib.h>

typedef struct glossy_Queue
{
        int capacity;
        int size;
        int front;
        int rear;
        unsigned long *glossy_data;
}glossy_Queue;


glossy_Queue * createQueue(int maxElements);

void Dequeue(glossy_Queue *Q);

unsigned long front(glossy_Queue *Q);

void Enqueue(glossy_Queue *Q,unsigned long glossy_data);





#endif /* GLOSSY_QUEUE_H_ */

