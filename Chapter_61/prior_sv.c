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
static void serviceReq(int cfd, int pfd) {
	char cookie[13]; // 12 byte cookie with space for newline
	ssize_t numRead;

	/* read cookie from normal sockets and autheticate this server
	   by returning cookie through priority socket */
	numRead = read(cfd, cookie, sizeof(cookie));
	if (numRead <= 0)
		_exit(EXIT_FAILURE);
	if (write(pfd, cookie, numRead) != numRead);
		_exit(EXIT_FAILURE);

		/******************************
		 * Communication happens here
		 * this could be done with poll
		 * and epoll
		 *****************************/

	 close(pfd);
	 close(cfd);
}

/**
 * Basic multi-process server for executing commands over a ip4 or ip6 socket
 */
int main() {
	int lfd, cfd, pfd;
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
		struct sockaddr_storage caddr;
		socklen_t len = sizeof(struct sockaddr_storage);
		char chost[20], pport[20];
		ssize_t numRead;

		/* returns a fully qualified client socket formed
		   by the lfd and peer's ephermeral socket */
		cfd = accept(lfd, (struct sockaddr *) &caddr, &len);
		if (cfd == -1) {
			perror("accept");
			continue;
		}

		// read port number of priority socket
		numRead = read(cfd, pport, sizeof(pport));
		if (numRead <= 0)
			continue;

		printf("numRead = %ld\n", numRead);
		// ensure null terminated
		pport[numRead] = '\0';

		// get the ipaddr of client and connect to the priority socket
		if (getnameinfo((struct sockaddr *) &caddr, len, chost, sizeof(chost),
				       	NULL, 0, NI_NUMERICHOST) == -1) {
			perror("getnameinfo");
			continue;
		}
		printf("Server connecting to priority socket %s:%s\n", chost, pport);
		pfd = inetConnect(chost, pport, SOCK_STREAM);	
		if (pfd == -1) {
			perror("inetConnect");
			continue;
		}

		/* Hanlde child requests concurrently */
		if (fork() == 0) {
			serviceReq(cfd, pfd);
			exit(EXIT_SUCCESS);
		}

		/* Close client connection, the child is handling it */
		close(cfd); 
		close(pfd);
	}
}
