/**
 * Same program that is in Chapter 53 except uses POSIX memory mapping instead
 * of a shared buffer
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <wait.h>
#include <semaphore.h>
#include <pthread.h>
#include <sys/mman.h>
#include <sys/stat.h>

#define MAP "/map"
#define BUF 1024

typedef struct {
	sem_t rsem;	
	sem_t wsem;
	int done;
	char buf[BUF];
} Map;

static Map *map;
/**
 * Reads from stdin and writes to memory map 
 */
static void writer() {
	ssize_t numRead;

	do {
		// get write semaphore
		if (sem_wait(&map->wsem) == -1) {
			perror("sem_wait writer");
			return;
		}
		numRead = read(STDIN_FILENO, map->buf, BUF);
		if (numRead == -1) {
			perror("read writer");
			map->done = 1;
			return;
		}

		// signal reader thread that we are finished
		if (numRead == 0)
			map->done = 1;

		if (sem_post(&map->rsem) == -1) {
			perror("sem_post writer");	
			return;
		}
	} while(numRead > 0);
}


/**
 * Reads from memory map and writes to stdout
 */
static void reader() {
	int stop = 0;

	while (!stop) {
		if (sem_wait(&map->rsem) == -1) {
			perror("sem_wait reader");
			return;
		}
		if (map->done)
			stop = 1;

		if (write(STDOUT_FILENO, &map->buf, BUF) == -1) {
			perror("write reader");
			return;		
		}

		if (sem_post(&map->wsem) == -1) {
			perror("sem_post reader");
			return;	
		}
	}
}

/**
 * Uses POSIX threads and semaphores to pipe stdin through a memory mapping in 1
 * thread and out from the buffer to stdout in another thread.
 */
int main(int argc, char *argv[]) {
	pthread_t tr, tw;

	// create memory mapping	
	umask(0);
	int fd = shm_open(MAP, O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
	if (fd == -1) { 
		perror("shm_open writer");
		return -1;
	}
	// resize to buffer size
	if (ftruncate(fd, sizeof(sem_t)*2 + BUF) == -1) {
		perror("ftruncate writer");
		return -1;		
	}

	map = mmap(NULL, BUF, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	if (map == MAP_FAILED) {
		perror("mmap");
		return -1;
	}
	map->done = 0;

	// init write semaphore to 1
	if (sem_init(&map->wsem, 1, 1) == -1) {
		perror("sem_init");
		return -1;
	}
	// init read semaphore to 0
	if (sem_init(&map->rsem, 1, 0) == -1) {
		perror("sem_init");
		return -1;
	}

	switch (fork()) {
		case -1:
			perror("fork");
			return -1;
		case 0:
			reader();
			exit(EXIT_SUCCESS);
		default:
			writer();
			break;
	}
	wait(NULL);	

	// clean up
	if (sem_destroy(&map->rsem) == -1) {
		perror("sem_destroy reader");
		return -1;	
	}
	if (sem_destroy(&map->wsem) == -1) {
		perror("sem_destroy writer");
		return -1;
	}
}
