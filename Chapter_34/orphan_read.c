#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

/**
 * Demonstrates how an orphaned process group cannot read from the controlling
 * terminal
 */
int main () {
	pid_t p = fork();
	char buf[1];
	
	switch(p) {
		case -1:
			perror("fork");
			break;
		case 0: // child
			sleep(1);
			/* ignore errors since this would be the  
			   only method of reporting them */
			read(STDIN_FILENO, buf, 1);
			FILE* fp = fopen("/dev/tty", "w");
			// should print Input output error (EIO)
			fprintf(fp, strerror(errno), 255); 
			fprintf(fp, "\n", 1);
			break;
		default:
			/* give children a chance to start and orphan 
			this is a race condition, but will be fine for
		       	for demo purposes */
			exit(EXIT_SUCCESS);
	}
	
}
