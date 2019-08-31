#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdlib.h>

/**
 * Demonstrates what happens to a grandchild process once its grandparent
 * has terminated. The child's parent is still the orignal parent after the 
 * grandparent has terminated, until the parent terminates and the init
 * process inherites the child.
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
			// wait for grand-parent to terminate
			sleep(5);
			printf("grand-parent PID: %d\n", getppid());
			
			p = fork();

			switch(p) {
				case -1:
					printf("error during fork\n");
					exit(EXIT_FAILURE);
					break;
				case 0: // child
					printf("child's parent PID: %d\n", getppid());
					sleep(5);
					printf("child's parent PID: %d\n", getppid());
					exit(EXIT_SUCCESS);
					break;
				default: // parent
					sleep(2);
					printf("parent exiting\n");
					break;
			}

			break;
		default: // parent
			printf("grand-parent PID: %d\n", getpid());
			printf("grand parent exiting...\n");
			_exit(EXIT_SUCCESS);
			break;
	}	
	
}
