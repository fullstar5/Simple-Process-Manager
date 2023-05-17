/*
 * This is entry point of program
 * This program tasks a file, read it and manage the processes inside the file
 * Two strategies provided by this program: SJF and RR
 * At the end of program, it will calculate Average turnaround time, 
 * Maximum TimeOverhead, Average TimeOverhead and Makespan
 *
 * Author: YiFeiZHANG 1174267
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
#include <sys/wait.h>
#include <sys/types.h>
#include <unistd.h>
#include <math.h>

#include "queue.h"
#include "task4.h"

#define IMPLEMENTS_REAL_PROCESS

 // SJF
void SJF(Node* ready_queue, Node* curr_running_node, int quantum){
	if (!isQueueEmpty(ready_queue)){
		curr_running_node -> timeRemain -= quantum;
	}
}

// RR
void RR(Node* ready_queue, Node* curr_running_node, int quantum){
	if (!isQueueEmpty(ready_queue)){
		curr_running_node -> timeRemain -= quantum;
	}	
}

// RR helper, switch to next process
Node* RRHelper_SwitchToNewOne(Node* ready_queue, int Ts){
	if (!isQueueEmpty(ready_queue)){
		Node* curr_running = deQueue(ready_queue);
		enQueue(ready_queue, curr_running);
		Node* new_running = ready_queue -> next;
		if (ready_queue -> data > 1){
			printf("%d, RUNNING, process_name=%s, remaining_time=%d\n",
				Ts, new_running -> processName, new_running -> timeRemain);
      	}
		return new_running;
	}
	return NULL;
}

// find a available memory hole
int findMeAHole(int i, int* bitMaps, int* hole_size, int* start_index, Node* target){
	int count = 0;
	int j;
	for (j = i; j < 2048; j++){
		if (bitMaps[j] == 0){
			count += 1;
		}
		else{
			if (count >= target -> memoryRequired){
				*hole_size = count;
				*start_index = j - count;
				return j;
			}
			count = 0;
		}
	}
	if (count >= target -> memoryRequired){
		*hole_size = count;
		*start_index = j - count;
		return j;
	}
	*hole_size = 99999;
	return -1;
}

// best fit strategy
int bestFit(int* bitMaps, Node* target){
	int best_size = 2048;
	int hole_size;
	int best_index = -1;
	int start_index = 0;
	int i = 0;
	while (i < 2048){
		i = findMeAHole(i, bitMaps, &hole_size, &start_index, target);
		// todo: if can't find one, break
		if (i == -1){
			break;
		}
		// if we find one, compare
		if (hole_size - target -> memoryRequired < best_size){
			best_size = hole_size - target -> memoryRequired;
			best_index = start_index;
		}
	}
	if (best_index == -1){
		return 0;
	}
	// occupied
	for (int j = start_index; j < start_index + target -> memoryRequired; j++){
		bitMaps[j] = 1;
	}
	target -> memoryStartIndex = start_index;
	return 1;
}

// memory allocation that invoke best fit strategy
void memoryAllocation(Node* input_queue, Node* ready_queue, int* bitMaps, int Ts){
	Node* curr = input_queue -> next;
	while (curr != NULL){
		int result = bestFit(bitMaps, curr);
		if (result == 1){
			Node* ready = deQueueByName(input_queue, curr);
			if (ready != NULL){
				curr = ready -> next;
				printf("%d,READY,process_name=%s,assigned_at=%d\n", 
						Ts, ready -> processName, ready -> memoryStartIndex);
				enQueue(ready_queue, ready);
			}

		}
		else{
			curr = curr -> next;
		}
	}
}

// memory deallocation when process finish
void memoryDeallocation(Node* funished, int* bitMaps){
	for (int i = funished -> memoryStartIndex; i < funished -> memoryStartIndex + funished -> memoryRequired; i++){
		bitMaps[i] = 0;
	}	
}




// ---------------------------------- main function ----------------------------------

int main(int argc, char* argv[]){

	int q = 0, Ts = 0, bitMaps [2048] = {0};
	char* s = NULL;
	char* m = NULL;

	double turnAroundTime = 0;
	double timeOverhead = 0;
	int makespan = 0;

	FILE* f = NULL;
	
	// read command line arguments
	for (int i = 1; i < argc; i+=2){
		if (i + 1 >= argc){
			fprintf(stderr, "Error: Missing value for option '%s'\n", argv[i]);
			exit(EXIT_FAILURE);
		}
		if (strcmp(argv[i], "-f") == 0){
			f = fopen(argv[i+1], "r");
		}
		else if (strcmp(argv[i], "-s") == 0){
			s = argv[i + 1];
		}
		else if (strcmp(argv[i], "-m") == 0){
			m = argv[i + 1];
		}
		else if (strcmp(argv[i], "-q") == 0){
			q = atoi(argv[i + 1]);
		}
		else{
			fprintf(stderr, "Error: Unknown option '%s'\n", argv[i]);
			exit(EXIT_FAILURE);
		}
	}
	if (f == NULL || s == NULL || m == NULL || q == 0) {
		fprintf(stderr, "Error: Missing required option(s)\n");
		exit(EXIT_FAILURE);
	}


	char buffer[2048];
	if (f == NULL){
		printf("open file error");
		exit(EXIT_FAILURE);
	}
	
	Node* waiting_queue = initQueue();
	Node* input_queue = initQueue();
	Node* ready_queue = initQueue();
	Node* funished_queue = initQueue();
	
	// read process in file into waiting_queue
	while (fgets(buffer, sizeof(buffer), f) != NULL){
		int arriveTime, serviceTime, memoryRequired;
		char* processName = (char*)malloc(sizeof(char) * 5);
		if (sscanf(buffer, "%d %s %d %d", &arriveTime, processName, &serviceTime, &memoryRequired) != 4){
			printf("something wrong with reading line data: %s\n", buffer);
		}
		Node* newNode = initQueue();
		newNode -> data = 0;
		newNode -> arriveTime = arriveTime;
		newNode -> processName = processName;
		newNode -> serviceTime = serviceTime;
		newNode -> timeRemain = serviceTime;
		newNode -> memoryRequired = memoryRequired;
		newNode -> memoryStartIndex = -1;
		newNode -> ran_before = 0;
		newNode -> pid = -99;
		enQueue(waiting_queue, newNode); 
	}
	fclose(f);
	
	int totalNum = waiting_queue -> data;  // record total number of processes
	Node* curr_running_node = NULL;
	if (strcmp(s, "SJF") == 0){
		
		while (funished_queue -> data < totalNum){
			if (curr_running_node != NULL){
				// step 1: identify whether current running process is funished (SJF)
				if (curr_running_node -> timeRemain <= 0){
					Node* funished = deQueueByName(ready_queue, curr_running_node);
					if (funished != NULL){
						if (strcmp(m, "best-fit") == 0){
							memoryDeallocation(funished, bitMaps);
						}  // release memory
						funished -> funishedTime = Ts;
						enQueue(funished_queue, funished);
						curr_running_node = NULL;
						printf("%d,FINISHED,process_name=%s,proc_remaining=%d\n", 
								Ts, funished->processName, ready_queue->data + input_queue->data);
						terminateProcess(funished, Ts);  // task 4 ************
						makespan = Ts;
					}
				}
			}
	
			// step 2: adding new arrive process to input queue
			while (!isQueueEmpty(waiting_queue)){
				if (waiting_queue -> next -> arriveTime > Ts){
					break;
				}
				Node* newArrive = deQueue(waiting_queue);
				enQueue(input_queue, newArrive);
			}
	
			// step 3: memory allocate
			if (strcmp(m, "best-fit") == 0){
				if (!isQueueEmpty(input_queue)){
					memoryAllocation(input_queue, ready_queue, bitMaps, Ts);
				}
			}
			else{
				while (!isQueueEmpty(input_queue)){
					Node* newAllocated = deQueue(input_queue);
					enQueue(ready_queue, newAllocated);
				}
			}
			

			// step 4: allocate new process
			if (curr_running_node == NULL){
				if (!isQueueEmpty(ready_queue)){
					curr_running_node = findMinServiceTime(ready_queue); 
					printf("%d,RUNNING,process_name=%s,remaining_time=%d\n",
						       	Ts, curr_running_node -> processName, curr_running_node -> timeRemain);

				}
			}
			if (curr_running_node != NULL){
				if (curr_running_node -> ran_before == 0){
					createProcess(curr_running_node, Ts);  // create process
					curr_running_node -> ran_before = 1;
				}
				else{
					continueRunProcess(curr_running_node, Ts);  // if process created, continue run process

				}
			}
	
			// step 5: run the SJF
			if (!isQueueEmpty(ready_queue)){
				SJF(ready_queue, curr_running_node, q);

			}
	
			// step 6: sleep
			Ts += q;
		}
	}

	if (strcmp(s, "RR") == 0){
		int RR_switcher = 0;
		while (funished_queue -> data < totalNum){
	
			// step 1: identify whether current running process is funished (RR)
			if (curr_running_node != NULL){
				if (curr_running_node -> timeRemain <= 0){
					Node* funished = deQueueByName(ready_queue, curr_running_node);
					if (funished != NULL){
						if (strcmp(m, "best-fit") == 0){
							memoryDeallocation(funished, bitMaps);
						}  // release memory
						funished -> funishedTime = Ts;
						enQueue(funished_queue, funished);
						printf("%d,FINISHED,process_name=%s,proc_remaining=%d\n", 
								Ts, funished -> processName, ready_queue->data + input_queue->data);
						terminateProcess(funished, Ts);
						makespan = Ts;
						// stop switching 
						RR_switcher = 0;
						curr_running_node = NULL;
					}
				}
			}



			// step 2: adding new arrive process to input queue
			while (!isQueueEmpty(waiting_queue)){
				if (waiting_queue -> next -> arriveTime > Ts){
					break;
				}
				Node* newArrive = deQueue(waiting_queue);
				enQueue(input_queue, newArrive);
			}
	
			// step 3: memory allocate
			if (strcmp(m, "best-fit") == 0){
				memoryAllocation(input_queue, ready_queue, bitMaps, Ts);
			}
			else{
				while (!isQueueEmpty(input_queue)){
					Node* newAllocated = deQueue(input_queue);
					enQueue(ready_queue, newAllocated);
				}
			}
			// suspend process
			if (curr_running_node != NULL && curr_running_node -> timeRemain > 0 && ready_queue -> data > 1){
				suspendProcess(curr_running_node, Ts);
			}

			// allocate memory
			if (RR_switcher != 0){
					if (!isQueueEmpty(ready_queue)){
						curr_running_node = RRHelper_SwitchToNewOne(ready_queue, Ts); // RR
				}
			}
			else{
				if (!isQueueEmpty(ready_queue)){
					curr_running_node = ready_queue -> next;
					printf("%d, RUNNING, process_name=%s, remaining_time=%d\n",
						Ts, curr_running_node -> processName, curr_running_node -> timeRemain);
					RR_switcher = 1;
				}
			}
			if (curr_running_node != NULL){
				if (curr_running_node -> ran_before == 0){
					createProcess(curr_running_node, Ts);
					curr_running_node -> ran_before = 1;
				}
				else{
					continueRunProcess(curr_running_node, Ts);
				}
			}
		

	
			// step 5: run the RR
			if (!isQueueEmpty(ready_queue)){
				if (curr_running_node != NULL){
					RR(ready_queue, curr_running_node, q);
				}
			}
	
			// step 6: sleep
			Ts += q;
		}
	}

// ------------------------------------------------------- printing and freeing ---------------------
	
	// calculate turnaround time, time overhead, makespan
	double maxTimeOverhead = -99999;
	Node* curr = funished_queue -> next;
	while (curr != NULL){
		double curr_turn = curr -> funishedTime - curr -> arriveTime;
		turnAroundTime += curr_turn;
		double curr_overhead = curr_turn / curr -> serviceTime;
		timeOverhead += curr_overhead;
		if (curr_overhead > maxTimeOverhead){
			maxTimeOverhead = curr_overhead;
		}
		curr = curr -> next;
	}
	
	turnAroundTime /= funished_queue -> data;
	timeOverhead  /= funished_queue -> data;
	printf("Turnaround time %d\n", (int)ceil(turnAroundTime));
	printf("Time overhead %.2f %.2f\n", maxTimeOverhead, timeOverhead);
	printf("Makespan %d\n", makespan);

	// free queue
	freeQueue(waiting_queue);
	freeQueue(input_queue);
	freeQueue(ready_queue);
	freeQueue(funished_queue);
	return 0;

}
