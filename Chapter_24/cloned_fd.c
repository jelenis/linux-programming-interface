#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <stdlib.h>

/**
 * Demonstrates how the file descriptors are copied
 * during a vfork, and thus closing the child's
 * will not affect the parent's
 */
int main (int argc, char* argv[]) {
	switch (vfork()) {
		case -1:
			perror("fork");
			return -1;
		case 0: // Child
			close(0);	
			exit(0);	
		default: 
			/* Parent can still use file descriptor since
			 they are copied */
			printf("hi\n");
	}
}
