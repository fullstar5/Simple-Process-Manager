/* 
 * This is implementation file for queue operations
 *
 * */

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

#include "queue.h"

// -------------- function defined -----------------------------------

Node* initQueue(){
	Node* queue = (Node*)malloc(sizeof(Node));
	assert(queue);
	queue -> data = 0;
	queue -> arriveTime = 0;
	//queue -> processName = (char*)malloc(sizeof(char) * 5);  // ***** temp change *****
	queue -> processName = NULL;
	queue -> serviceTime = 9999;
	queue -> timeRemain = 9999;
	queue -> funishedTime = 0;
	queue -> memoryRequired = 0;
	queue -> memoryStartIndex = -1;
	queue -> next = NULL;
	queue -> ran_before = 0;
	return queue;
}

int isQueueEmpty(Node* queue){
	if (queue -> data > 0){
		return 0;
	}
	else{
		return 1;
	}
}

void enQueue(Node* Q, Node* q){
	Node* temp = Q;
	for (int i = 0; i < Q -> data; i++){
		temp = temp -> next;
	}
	q -> next = temp -> next;
	temp -> next = q;
	Q -> data++;
}

Node* deQueue(Node* Q){
	if (isQueueEmpty(Q)){
		//printf("no nodes in queue\n");
		return NULL;
	}
	else{
		Node* node = Q -> next;
		Q -> next = Q -> next -> next;
		Q -> data--;
		return node;
	}
}

void freeQueue(Node* head){
	Node* curr = head;
	while (curr != NULL){
		Node* next = curr -> next;
		free(curr -> processName);
		free(curr);
		curr = next;
	}
}


Node* findMinServiceTime(Node* Q){
	Node* marker;
	Node* curr = Q -> next;
	int currentMin = 99999;
	while (curr != NULL){
		if (curr -> serviceTime < currentMin){
			marker = curr;
			currentMin = curr -> serviceTime;
		}
		curr = curr -> next;
	}
	return marker;
}


Node* deQueueByName(Node* Q, Node* target){
	if (!isQueueEmpty(Q)){	
		Node* curr = Q;
		while (curr -> next != NULL){
			if (strcmp(curr -> next -> processName, target -> processName) == 0){
				Node* temp = target;
				curr -> next = temp -> next;
				Q -> data--;
				return temp;
			}
			curr = curr -> next;
		}
		return NULL;
	}
	return NULL;
}


void printQueue(Node* Q){
	printf("this queue has %d nodes\n", Q -> data);
	Node* node = Q -> next;
	printf("Process Name: ");
	while (node){
		printf("%s -> ", node -> processName);
		node = node -> next;
	}
	printf("NULL\n");
	node = Q -> next;
	printf("Arrive Time: ");
	while (node){
		printf("%d -> ", node -> arriveTime);
		node = node -> next;
	}
	printf("NULL\n");
	node = Q -> next;
	printf("Service Time: ");
	while (node){
		printf("%d -> ", node -> serviceTime);
		node = node -> next;
	}
	printf("NULL\n");
}
