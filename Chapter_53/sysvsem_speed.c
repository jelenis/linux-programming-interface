#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/sem.h>
#include "semun.h"

/**
 * Speed test of System V semaphores, ignores error checking.
 * This will be signifigantly slower than POSIX semaphores if 
 * there is a large number of increment & decrement operations
 * (wait and post).
 */
int main (int argc, char *argv[]) {
	unsigned long int times = 0;
	union semun arg;
	struct sembuf sop;
	if (argc > 1) {
		times = strtoul(argv[1], NULL, 10);
	}

	// create System V semaphore and init to 1
	int semid = semget(IPC_PRIVATE, 1, IPC_CREAT | S_IRUSR | S_IWUSR);
	arg.val = 1;
	semctl(semid, 0, SETVAL, arg);
	sop.sem_flg = 0;
	sop.sem_num = 0;

	for (unsigned long int i= 0; i < times; i++) {
		sop.sem_op = -1;
		semop(semid, &sop, 1);
		sop.sem_op = 1;
		semop(semid, &sop, 1);
	}

}
