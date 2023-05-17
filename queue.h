/*
 * This is header file for queue operations
 *
 *
 * */

#ifndef QUEUE_H
#define QUEUE_H

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <err.h>
#include <getopt.h>
#include <signal.h>
#include <sys/file.h>
#include <sys/signalfd.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <math.h>



typedef struct Node{
	int data;
	int arriveTime;
	char* processName;
	int serviceTime;
	int timeRemain;
	int funishedTime;
	int memoryRequired;
	int memoryStartIndex;
	struct Node* next;
	int pipe1[2];
	int pipe2[2];
	int pid;
	int ran_before;
}Node;

Node* initQueue();
int isQueueEmpty(Node* queue);

void enQueue(Node* Q, Node* q);

Node* deQueue(Node* Q);

void freeQueue(Node* head);

Node* findMinServiceTime(Node* Q);

Node* deQueueByName(Node* Q, Node* target);

void printQueue(Node* Q);

#endif
