#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>

/**
 * Demonstrates the time difference in fork vs vfork. Timing should be performed
 * using the shell command time
 *
 * Add any command line arguemnt to use vfork
 */
int main(int argc, char* argv[]) {
	pid_t p;

	// create child multiple times to see time difference
	for (int i = 0; i < 100; i++) {
		if (argc > 1)
			p = vfork();
		else 
			p = fork();
		switch (p) {
			case 0:
				_exit(EXIT_SUCCESS);	
			default:
				wait(NULL);
		}
	}		

	return 0;
}
