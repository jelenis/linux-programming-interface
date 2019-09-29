/**
 * Client to that has two sockets, 1 for priority messages the other for normal data
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "inet_sockets.h"
#include <string.h>
#include <time.h>

#define SERVICE "50000"
#define BACK_LOG 10

int main (int argc, char *argv[]) {
	char buf[256];
	char lport[22];
	ssize_t port_len;
	int nfd, lfd, pfd;
	struct sockaddr_in laddr;
	char cookie[13];
	socklen_t len = sizeof(struct sockaddr_in);
	memset(&laddr, 0, sizeof(laddr));
	
	if (argc < 2) {
		printf("Enter a host\n");
		return -1;
	}

	/* Create a listening socket without binding, this will let the kernel
	   decide an appropriate port, use ip4 for brevity */
	lfd = socket(AF_INET, SOCK_STREAM, 0);
	if (lfd == -1) {
		perror("socket");	
		return -1;
	}
	if (listen(lfd, BACK_LOG) == -1) {
		perror("listen");
		return -1;
	}
	if (getsockname(lfd, (struct sockaddr *) &laddr, &len) == -1) {
		perror("getsockname");
		return -1;
	}
	if (getnameinfo((struct sockaddr *) &laddr, len,
			       	NULL, 0, lport, sizeof(lport), NI_NUMERICSERV) == -1) {
		perror("getsockname");
		return -1;
	}
	printf("listening on %s\n", lport);
	nfd = inetConnect(argv[1], SERVICE, SOCK_STREAM);
	if (nfd == -1) {
		perror("inetConnect");
		return -1;
	}
	printf("%ld\n", strlen(lport));
	if (write(nfd, lport, strlen(lport)) != strlen(lport)) {
		perror("write");
		return -1;	
	}
	pfd = accept(lfd, NULL, NULL);
	
	// create authentication cookie	
	srand((unsigned	int) time(NULL));
	snprintf(cookie, sizeof(cookie), "%d%d%d", rand(), rand(), rand());

	// authenticate priority socket 
	if (write(nfd, cookie, strlen(cookie)) != strlen(cookie)) {
		perror("write");
		return -1;
	}
	if (read(pfd, buf, strlen(cookie)) != strlen(cookie)) {
		fprintf(stderr, "Socket connection could be authenticated\n");
		return -1;
	}
	if (strcmp(buf, cookie) != 0) {
		fprintf(stderr, "Socket connection could be authenticated\n");
		return -1;
	}
	printf("authenticated\n");
	

 	//close the write half to signal server that the command is done sending 
	//if (shutdown(nfd, SHUT_WR) == -1) {
		//perror("shutdown");
		//return -1;
	//}
		
}
