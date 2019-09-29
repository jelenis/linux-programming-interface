/**
 * Simple server that allows clients to execute commands locally
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "inet_sockets.h"
#include <sys/wait.h>
#include <signal.h>
#include <errno.h>

#define SERVICE "50000"
#define BACK_LOG 10

/**
 * Kills off any zombie process 
 */
static void reaper(int signum) {
	int e = errno;
	while(waitpid(-1, NULL, WNOHANG) > 0)
		continue;
	errno = e;
}

/**
 * Handle requests concurrently in a child process
 * reads a command from fd and waits for the peer to close the write socket
 * (ie send EOF)
 * then executes the command and redirects its output to the socket
 */
static void serviceReq(int fd) {
	char buf[1024];

	 while(read(fd, buf, sizeof(buf)) > 0)
	 	continue;

	 // redirect stdout -> socket
	 if (dup2(fd, STDOUT_FILENO) == -1) {
		 perror("dup2");
		 _exit(EXIT_FAILURE);
	 }

	 if (system(buf) == -1) {
		 perror("system");
		 _exit(EXIT_FAILURE);
	 }
	 close(fd);
}

/**
 * Basic multi-process server for executing commands over a ip4 or ip6 socket
 */
int main() {
	int lfd, cfd;
	struct sigaction sa;

	/* establish child handler */
	sigemptyset(&sa.sa_mask); // do not block other signals
	sa.sa_handler = reaper;
	sa.sa_flags = SA_RESTART; // restart system calls
	if (sigaction(SIGCHLD, &sa, NULL) == -1) {
		perror("sigaction");
		exit(EXIT_FAILURE);
	}
	

	lfd = inetListen(SERVICE, BACK_LOG, NULL);
	if (lfd == -1) {
		perror("lfd");
		exit(EXIT_FAILURE);
	}


	/* loop accepting client connections  */
	for (;;) {
		/* returns a fully qualified client socket formed
		   by the lfd and peer's ephermeral socket */
		cfd = accept(lfd, NULL, NULL);
		if (cfd == -1) {
			perror("accept");
			continue;
		}
		
		/* Hanlde child requests concurrently */
		if (fork() == 0) {
			serviceReq(cfd);
			exit(EXIT_SUCCESS);
		}

		/* Close client connection, the child is handling it */
		close(cfd); 
	}
}
