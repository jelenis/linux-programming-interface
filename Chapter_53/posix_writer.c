#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <semaphore.h>
#include <pthread.h>

static sem_t rsem;	
static sem_t wsem;
static char buf[1024];
static int done = 0;
/**
 * Reads from stdin and writes to global buf
 */
static void* writer(void *arg) {
	ssize_t numRead;
	do {
		// get write semaphore
		if (sem_wait(&wsem) == -1) {
			perror("sem_wait writer");
			return NULL;
		}
		numRead = read(STDIN_FILENO, buf, 1024);
		if (numRead == -1) {
			perror("read writer");
			done = 1;
			return NULL;
		}

		// signal reader thread that we are finished
		if (numRead == 0)
			memset(buf, 0, 1024);
			//done = 1;

		if (sem_post(&rsem) == -1) {
			perror("sem_post writer");	
			return NULL;
		}
	} while(numRead > 0);
}


/**
 * Reads from global buffer and writes to stdout
 */
static void* reader(void *arg) {
	int stop = 0;

	while (!stop) {
		if (sem_wait(&rsem) == -1) {
			perror("sem_wait reader");
			return NULL;
		}
		if (done)
			stop = 1;

		if (write(STDOUT_FILENO, buf, 1024) == -1) {
			perror("write reader");
			return NULL;		
		}

		if (sem_post(&wsem) == -1) {
			perror("sem_post reader");
			return NULL;	
		}
	}
}

/**
 * Uses POSIX threads ans semaphores to pipe stdin through a buffer in 1
 * thread and out from the buffer to stdout in another thread.
 */
int main(int argc, char *argv[]) {
	pthread_t tr, tw;

	// init write semaphore to 1
	if (sem_init(&wsem, 0, 1) == -1) {
		perror("sem_init");
		return -1;
	}
	// init read semaphore to 0
	if (sem_init(&rsem, 0, 0) == -1) {
		perror("sem_init");
		return -1;
	}

	if (pthread_create(&tw, NULL, writer, NULL) == -1) {
		perror("writer create");
		return -1;
	}
	if (pthread_create(&tr, NULL, reader, NULL) == -1) {
		perror("writer create");
		return -1;
	}

	// wait for both threads to finish
	if (pthread_join(tw, NULL) != 0) {
		perror("ptread_join writer");
		return -1;
	}
	if (pthread_join(tr, NULL) != 0) {
		perror("ptread_join reader");
		return -1;
	}

	// clean up
	if (sem_destroy(&rsem) == -1) {
		perror("sem_destroy reader");
		return -1;	
	}
	if (sem_destroy(&wsem) == -1) {
		perror("sem_destroy writer");
		return -1;
	}
}
