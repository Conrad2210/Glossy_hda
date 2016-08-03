/*
 * glossy_queue.c
 *
 *  Created on: Jun 16, 2016
 *      Author: user
 */


#include "glossy_queue.h"

//struct queueCDT *queueADT;

void QueueInit(queueCDT* queueADT)
{
  int i = 0;
  queueADT->front = -1;
  queueADT->rear = -1;
  queueADT->count = 0;
  for(i=0;i<MAX_QUEUE_SIZE;i++)
    queueADT->glossy_data[i].seq_no = 0;
}

 void Enqueue(queueCDT* queueADT, glossy_data_struct data){
   queueADT->rear = (queueADT->rear+1)%MAX_QUEUE_SIZE;
   queueADT->glossy_data[queueADT->rear] = data;
   queueADT->count++;
 }

 void Dequeue(queueCDT* queueADT){
   queueADT->rear = -1;
   queueADT->count--;
 }


 glossy_data_struct getGlossyData(queueCDT* queueADT){
   return queueADT->glossy_data[++queueADT->front%MAX_QUEUE_SIZE];
 }

int get_count(queueCDT* queueADT)
{
return queueADT->count;
}
