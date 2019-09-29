/**
 * Simple client to executre commands on a server
 * Usage: ./cmd_cl host-name "command to execute"
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "inet_sockets.h"
#include <string.h>

#define SERVICE "50000"

int main (int argc, char *argv[]) {
	char buf[256];
	int cfd;
	ssize_t len;	

	if (argc < 3) {
		printf("Enter a host followed by a command\n");
		return -1;
	}


	cfd = inetConnect(argv[1], SERVICE, SOCK_STREAM);
	if (cfd == -1) {
		perror("inetConnect");
		return -1;
	}

	len = strlen(argv[2]);
	if (write(cfd, argv[2], len) != len) {
		perror("write");
		return -1;	
	}

	// close the write half to signal server that the command is done sending 
	if (shutdown(cfd, SHUT_WR) == -1) {
		perror("shutdown");
		return -1;
	}

	// read output of the command 
	while(read(cfd, buf, sizeof(buf)) > 0)
		printf("%s", buf);
	
}
