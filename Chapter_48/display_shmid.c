#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <sys/types.h>


/**
 * Simply displays info about the shared System V memory in the command line
 * argument
 */
int main(int argc, char* argv[]) {
	int shmid;

	if (argc < 2) {
		printf("Please enter shared memory segment id\n");
		return -1;
	}

	shmid  = strtod(argv[1], NULL);

	struct shmid_ds ds;
	if (shmctl(shmid, IPC_STAT, &ds) == -1) {
		perror("shmctl");	
		return -1;
	}

	printf("SIZE\t\tCREATOR\t\tNo. Attach\n");
	printf("%d\t\t%d\t\t%d\n",ds.shm_segsz, ds.shm_cpid, ds.shm_nattch);

}
