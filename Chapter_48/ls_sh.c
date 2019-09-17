#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <sys/types.h>
#include <errno.h>

/**
 * Lists all the System V shared memory mappings
 */
int main(int argc, char* argv[]) {
	int shmid, max;
	struct shmid_ds ds;
	struct shminfo info;

	// get  maximum index of memory segments
	max = shmctl(shmid, IPC_INFO, (struct shmid_ds*)&info);
	if (max == -1) {
		perror("shmctl");	
		return -1;
	}

	for (int i=0; i < max; i++) {
		if (shmctl(i, IPC_STAT, &ds) == -1) {
			if (errno == EINVAL)
				continue; // index not used
			// exit on unexpected error
			perror("shmctl");
			return -1;
		}

		printf("SIZE\t\tCREATOR\t\tNo. Attach\n");
		printf("%d\t\t%d\t\t%d\n",ds.shm_segsz, ds.shm_cpid, ds.shm_nattch);
	}


}
