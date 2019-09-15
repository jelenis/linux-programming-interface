/* 
 * Implmentation of the VMS Event API using System V semaphores
 */
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/sem.h>
#include <signal.h>
#include <unistd.h>
#include <fcntl.h>
#include "semun.h"

/**
 * Sets the event flag
 * changes the value of the System V sempahore to 0
 */
int setEventFlag(int semId, int semNum) {
	union semun arg;
	arg.val = 0;
	return semctl(semId, semNum, SETVAL, arg);	
}


/**
 * removes the 
 */
int clearEventFlag(int semId, int semNum) {
	union semun arg;
	arg.val = 1;
	return semctl(semId, semNum, SETVAL, arg);	
}

/**
 * Blocks until the event is triggered.
 * Waits on the System V semaphore to be 0, if a signal interrupts
 * this call, will continue to wait on the event being set.
 */
int waitForEventFlag(int semId, int semNum) {
	struct sembuf sop;
	sop.sem_num =  semNum;
	sop.sem_op = 0;
	sop.sem_flg = 0;

	while (semop(semId, &sop, 1) == -1)
		if (errno != EINTR)
			return -1;
	return 0;
}


/**
 * Returns 1 if the event is set, else 0
 */
int getFlagState(int semId, int semNum) {
	return semctl(semId, semNum, GETVAL);	
}

/**
 * Wrapper to create VMS event
 */
int initEvent() {
	int semid = semget(IPC_PRIVATE, 1, IPC_CREAT | S_IRUSR | S_IWUSR);
	if (semid == -1) {
		return -1;
	}
	if (clearEventFlag(semid, 0) == -1) {
		return -1;
	}	
     return semid;	
}

/**
 * Driver program to test this API.
 */
int semid;

static void handler(int signum) {
	if (setEventFlag(semid, 0) == -1) {
		perror("setEventFlag");
		exit(EXIT_FAILURE);
	}
}

int main() {
	struct sigaction sa;
	sa.sa_handler = handler;
	sa.sa_flags = 0;
	if (sigaction(SIGALRM, &sa, NULL) == -1) {
		perror("sigaction");
		return -1;
	}
	semid = initEvent();
	alarm(3);
	printf("about to wait on event\n");
	if (waitForEventFlag(semid, 0) == -1) {
		perror("waitForEventFlag");
		return -1;	
	}
	printf("done waiting for event\n");

}
