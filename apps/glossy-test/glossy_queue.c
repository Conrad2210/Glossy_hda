/*
 * glossy_queue.c
 *
 *  Created on: 27-Jun-2016
 *      Author: NithinTomy
 */

#include"glossy_queue.h"

glossy_Queue * createQueue(int maxElements)
{
	/* Create a Queue */
	        glossy_Queue *Q;
	        Q = (glossy_Queue *)malloc(sizeof(glossy_Queue));
	        /* Initialise its properties */
	        Q->glossy_data = (unsigned long *)malloc(sizeof(int)*maxElements);
	        Q->size = 0;
	        Q->capacity = maxElements;
	        Q->front = 0;
	        Q->rear = -1;
	        /* Return the pointer */
	        return Q;


}
void Dequeue(glossy_Queue *Q)
{


    /* If Queue size is zero then it is empty. So we cannot pop */
    if(Q->size==0)
    {
            printf("Queue is Empty\n");
            return;
    }
    /* Removing an element is equivalent to incrementing index of front by one */
    else
    {
            Q->size--;
            Q->front++;
            /* As we fill elements in circular fashion */
            if(Q->front==Q->capacity)
            {
                    Q->front=0;
            }
    }
    return;
}

unsigned long front(glossy_Queue *Q)
{

	 if(Q->size==0)
	        {
	                printf("Queue is Empty\n");
	                exit(0);
	        }
	        /* Return the element which is at the front*/
	        return Q->glossy_data[Q->front];

}

void Enqueue(glossy_Queue *Q,unsigned long glossy_data)
{
	 /* If the Queue is full, we cannot push an element into it as there is no space for it.*/
	        if(Q->size == Q->capacity)
	        {
	                printf("Queue is Full\n");
	        }
	        else
	        {
	                Q->size++;
	                Q->rear = Q->rear + 1;
	                /* As we fill the queue in circular fashion */
	                if(Q->rear == Q->capacity)
	                {
	                        Q->rear = 0;
	                }
	                /* Insert the element in its rear side */
	                Q->glossy_data[Q->rear] = glossy_data;
	        }
	        return;


}


