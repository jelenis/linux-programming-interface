#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdlib.h>

/**
 * Demonstrates that the init process will inherit the parentless child process
 */
int main() {
	
	printf("creating child\n");

	pid_t p  = fork();

	switch(p) {
		case -1:
			printf("error during fork\n");
			exit(EXIT_FAILURE);
			break;
		case 0: // child
			// wait for parent to terminate
			sleep(5);
			printf("parent PID: %d\n", getppid());
			break;
		default: // parent
			printf("parent exiting...\n");
			_exit(EXIT_SUCCESS);
			break;
	}	
	
}
