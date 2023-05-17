/*
 * This file is for task4 header
 *
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


void createProcess(Node* p, int Ts);
void continueRunProcess(Node* p, int Ts);
void suspendProcess(Node* p, int Ts);
void terminateProcess(Node* p, int Ts);
