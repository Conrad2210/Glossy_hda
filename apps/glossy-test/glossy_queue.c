/*
 * glossy_queue.c
 *
 *  Created on: Jun 16, 2016
 *      Author: user
 */


#include "glossy_queue.h"

struct queueCDT *queueADT;

void QueueInit()
{
  int i = 0;
  queueADT->front = -1;
  queueADT->rear = -1;
  for(i=0;i<MAX_QUEUE_SIZE;i++)
    queueADT->glossy_data[i].seq_no = 0;
}

 void Enqueue(glossy_data_struct data){
   queueADT->rear = (queueADT->rear+1)%MAX_QUEUE_SIZE;
   queueADT->glossy_data[queueADT->rear] = data;
 }

 void Dequeue(){
   queueADT->rear = -1;
 }

 bool IsEmpty(){
   if ((queueADT->rear == -1) && (queueADT->front == -1))
     return true;
   else
     return false;
 }

 bool IsFull(){
   if (queueADT->rear == MAX_QUEUE_SIZE-1)
      return true;
    else
      return false;
 }

 glossy_data_struct getGlossyData(){
   return queueADT->glossy_data[++queueADT->front%MAX_QUEUE_SIZE];
 }
