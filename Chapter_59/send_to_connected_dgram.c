#include "inet_sockets.h"
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <arpa/inet.h>

/**
 * Demonstrates what happens when a UDP socket sends a datagram to a 
 * server if a another socket is already connected to the server
 */
int main() {
	int fd1, fd2, fd3;	

	fd1 = inetBind("50000", SOCK_DGRAM, NULL);
	if (fd1 == -1) {
		perror("inetBind");
		return -1;
	}
        fd2 = inetBind("50001", SOCK_DGRAM, NULL);
	if (fd2 == -1) {
		perror("inetBind");
		return -1;
	}

	// using ipv4, not working locally with ipv6
	struct sockaddr_in svaddr;
	memset(&svaddr, 0, sizeof(struct sockaddr_in));
	svaddr.sin_family = AF_INET;
	svaddr.sin_port = htons(50001);
	if (inet_pton(AF_INET, "localhost", &svaddr.sin_addr) == -1) {
		perror("inet_pton");
		return -1;
	}

	// connect the first socket to the second address 
	if (connect(fd1, (struct sockaddr *) &svaddr,
			       	sizeof(struct sockaddr_storage)) == -1) {
		perror("connect");
		return -1;
	}

	fd3 = inetBind("50002", SOCK_DGRAM, NULL);
	if (fd3 == -1) {
		perror("inetBind");
		return -1;
	}
	// now send from unconnected socket
	if (sendto(fd3, "h", 1, 0, (struct sockaddr *) &svaddr,
			       	sizeof(struct sockaddr_storage)) != 1) {
		// should fail with already in use
		perror("sendto");
		return -1;
	}

}


