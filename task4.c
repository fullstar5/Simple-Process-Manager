/*
 * This is task4 implementation file
 * Author: yifeizhang
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



void createProcess(Node* p, int Ts){
	pid_t pid;
	if (pipe(p -> pipe1) == -1){
		perror("error when create pipe1");
		exit(EXIT_FAILURE);
	}
	if (pipe(p -> pipe2) == -1){
		perror("error when create pipe2");
		exit(EXIT_FAILURE);
	}

	// create child process
	pid = fork();
	if (p -> pid == -1){
		perror("error when fork");
		exit(EXIT_FAILURE);
	}
	// this is child process
	else if (pid == 0){
		close(p -> pipe1[1]);
		close(p -> pipe2[0]);

		dup2(p -> pipe1[0], STDIN_FILENO);
		dup2(p -> pipe2[1], STDOUT_FILENO);

		char* args[] = {"-v", p -> processName, NULL};
		execvp("./process", args);
	}
	else{
		p -> pid = pid;
		close(p -> pipe1[0]);
		close(p -> pipe2[1]);

		unsigned char tmp[4];
		tmp[0] = (Ts >> 24) & 0xff;
		tmp[1] = (Ts >> 16) & 0xff;
		tmp[2] = (Ts >> 8) & 0xff;
		tmp[3] = Ts & 0xff;

		write(p -> pipe1[1], tmp, sizeof(tmp));

		unsigned char c;
		read(p -> pipe2[0], &c, sizeof(c));
		if (tmp[3] != c){
			printf("wrong message you passing whe creating process\n");
		}
	}
}



void continueRunProcess(Node* p, int Ts){
	unsigned char tmp[4];
	tmp[0] = (Ts >> 24) & 0xff;
	tmp[1] = (Ts >> 16) & 0xff;
	tmp[2] = (Ts >> 8) & 0xff;
	tmp[3] = Ts & 0xff;

	write(p -> pipe1[1], tmp, sizeof(tmp));
	kill(p -> pid, SIGCONT);

	unsigned char c;
	read(p -> pipe2[0], &c, 1);
	if (tmp[3] != c){
		printf("the message did NOT match when running\n");
	}
}



void suspendProcess(Node* p, int Ts){
	unsigned char tmp[4];
	tmp[0] = (Ts >> 24) & 0xff;
	tmp[1] = (Ts >> 16) & 0xff;
	tmp[2] = (Ts >> 8) & 0xff;
	tmp[3] = Ts & 0xff;

	write(p -> pipe1[1], tmp, sizeof(tmp));
	int wstatus;
	kill(p -> pid, SIGTSTP);
	waitpid(p -> pid, &wstatus, WUNTRACED);

	while (!WIFSTOPPED(wstatus)){;}
}

void terminateProcess(Node* p, int Ts){
	unsigned char tmp[4];
	tmp[0] = (Ts >> 24) & 0xff;
	tmp[1] = (Ts >> 16) & 0xff;
	tmp[2] = (Ts >> 8) & 0xff;
	tmp[3] = Ts & 0xff;

	write(p -> pipe1[1], tmp, sizeof(tmp));
	kill(p -> pid, SIGTERM);

	char buffer[65];
	read(p -> pipe2[0], buffer, 65);

	printf("%d,FINISHED-PROCESS,process_name=%s,sha=", Ts, p -> processName);
	for (int i = 0; i < 65; i++){
		if (buffer[i] != '\n'){
			printf("%c", buffer[i]);
		}
	}
	printf("\n");
	close(p -> pipe1[1]);
	close(p -> pipe2[0]);
}
