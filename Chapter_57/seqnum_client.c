#include "seqnum.h"
#include <unistd.h>
#include <stdlib.h>

/**
 * Requests server to allocate a sequence of numbers using UDP UNIX sockets.
 * The address of the client socket is abstract (does not exist in the file system)
 */
int main(int argc, char *argv[]) {
	int sfd, req, resp;
	struct sockaddr_un svaddr, claddr;

	if (argc < 2) {
		fprintf(stderr, "Missing seqeunce length");
		return -1;
	} 
	req = strtod(argv[1], NULL);

	// create abstract known server address by leaving the first character null
	memset(&svaddr, 0, sizeof(struct sockaddr_un));
	strncpy(&svaddr.sun_path[1], SV_SOCK, strlen(SV_SOCK));
	svaddr.sun_family = AF_UNIX;
	
	// create abstract client address using pid
	memset(&claddr, 0, sizeof(struct sockaddr_un));
	sprintf(claddr.sun_path, "\0%ld", getpid());
	claddr.sun_family = AF_UNIX;

	/* create socket and bind to abstract address */
	sfd = socket(AF_UNIX, SOCK_DGRAM, 0);
	if (sfd == -1) {
		perror("socket");
		return -1;	
	}
	if (bind(sfd, (struct sockaddr *) &claddr, sizeof(struct sockaddr_un)) == -1) {
		perror("bind");
		return -1;
	}

	socklen_t len = sizeof(struct sockaddr_un);
	if (sendto(sfd, &req, sizeof(req), 0,  (struct sockaddr *) &svaddr, len) != sizeof(req)) {
		perror("sendto");
		return -1;
	}	
	size_t numBytes = recvfrom(sfd, &resp, sizeof(resp), 0,
			(struct sockaddr *) &claddr, &len);
	if (numBytes != sizeof(resp)) {
		perror("recvfrom");
		return -1;
	}		

	printf("%d\n", resp);
}
