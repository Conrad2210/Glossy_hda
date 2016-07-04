/*
 * glossy_queue.h
 *
 *  Created on: Jun 16, 2016
 *      Author: Nithin Raj
 */

#ifndef GLOSSY_QUEUE_H_
#define GLOSSY_QUEUE_H_

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#define MAX_QUEUE_SIZE  40

typedef struct {
  unsigned long seq_no; /**< Sequence number, incremented by the initiator at each Glossy phase. */
} glossy_data_struct;

typedef struct queueCDT {
  glossy_data_struct glossy_data [MAX_QUEUE_SIZE];
  int front;
  int rear;
  int count;

 } queueCDT;


void QueueInit();
void Enqueue(glossy_data_struct data);
void Dequeue();
bool IsEmpty();
bool IsFull();
glossy_data_struct getGlossyData();
#endif /* GLOSSY_QUEUE_H_ */
