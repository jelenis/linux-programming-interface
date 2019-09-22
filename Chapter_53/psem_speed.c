#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <semaphore.h>

/**
 * Speed test of POSIX semaphores, ignores error checking.
 * This will be signifigantly faster than System V sempahores when
 * there are lots of wait & post (increment & decrement) operations.
 */
int main (int argc, char *argv[]) {
	unsigned long int times = 0;
	if (argc > 1) {
		times = strtoul(argv[1], NULL, 10);
	}

	// create unamed posix semaphore
	sem_t sem;
	sem_init(&sem, 0, 1);	

	for (unsigned long int i= 0; i < times; i++) {
		sem_wait(&sem);
		sem_post(&sem);
	}

}
