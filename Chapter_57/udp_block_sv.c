#include "un.h"

/**
 * Consumes client datagrams slowly causing the client to block on send.
 */
int main(int argc, char *argv[]) {
	struct sockaddr_un svaddr;
	int sfd;

	/* Create UNIX UDP socket */
	sfd = socket(AF_UNIX, SOCK_DGRAM, 0);
	if (sfd == -1) {
		perror("socket");
		return -1;
	}
	
	// init struct fields
	memset(&svaddr, 0, sizeof(struct sockaddr_un));
	svaddr.sun_family = AF_UNIX;
	// create abstract UDP address (first byte in sun_path is null)
	strncpy(&svaddr.sun_path[1], SV_SOCK, strlen(SV_SOCK));
	
	/* Bind the socket to the abstract UDP address */
	if (bind(sfd, (struct sockaddr*) &svaddr, sizeof(struct sockaddr_un)) == -1) {
		perror("bind");
		return -1;
	}

	for (;;) {
		char buf[BUF_SIZE];
		ssize_t numBytes = recv(sfd, buf, BUF_SIZE, 0);
		if (numBytes == -1) {
			perror("recv");
			return -1;
		}
		printf("Received %ld bytes: %s\n", numBytes, buf);

		// slowly consume datagrams from client
		sleep(20); 
	}

}
