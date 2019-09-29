#include <stdio.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>
#include <wait.h>

/**
 * Pipe implmentation using socketpair
 */
int pipe(int filedes[2]) {
	if (socketpair(AF_UNIX, SOCK_STREAM, 0, filedes) == -1)
		return -1;

	// close writing on the read end
	if (shutdown(filedes[0], SHUT_WR) == -1)
		return -1;

	// close reading on the write end
	if (shutdown(filedes[1], SHUT_RD) == -1)
		return -1;

}

/**
 * Driver program to test pipe implementation
 */
int main () {
	int filedes[2];	
	char buf[1];

	if (pipe(filedes) == -1) {
		perror("pipe");
		return -1;
	}
	
	if (fork() == 0) {
		read(filedes[0], buf, 1);
		printf("read %c\n", buf[0]);
		_exit(EXIT_SUCCESS);
	} else {
		write(filedes[1], "x", 1);
		wait(NULL);
	}

}
