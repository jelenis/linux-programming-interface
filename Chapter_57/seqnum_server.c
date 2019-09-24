#include "seqnum.h"

/**
 * Allocates a sequence of numbers to a client using UDP UNIX sockets.
 * The address of the server socket is abstract (does not exist in the file system)
 */
int main() {
	int seqNum = 0, sfd;
	struct sockaddr_un svaddr, claddr;

	memset(&svaddr, 0, sizeof(struct sockaddr_un));
	svaddr.sun_family = AF_UNIX;
	
	// copy from sun_path[1] leaving the first character null
	strncpy(&svaddr.sun_path[1], SV_SOCK, strlen(SV_SOCK));

	/* create socket and bind to abstract address */
	sfd = socket(AF_UNIX, SOCK_DGRAM, 0);
	if (sfd == -1) {
		perror("socket");
		return -1;	
	}
	if (bind(sfd, (struct sockaddr *) &svaddr, sizeof(struct sockaddr_un)) == -1) {
		perror("bind");
		return -1;
	}
	
	printf("Server started\n");
	for (;;) {
		int req;
		socklen_t len = sizeof(struct sockaddr_un);
		size_t numBytes = recvfrom(sfd, &req, sizeof(req), 0,
			       	(struct sockaddr *) &claddr, &len);
		if (numBytes == -1) {
			perror("recvfrom");
			return -1;
		}		
		printf("Requested %d\n", req);

		if (sendto(sfd, &seqNum, sizeof(seqNum), 0,  (struct sockaddr *) &claddr, len) != sizeof(seqNum)) {
			perror("sendto");
			return -1;
		}	
		printf("Sequence starts at %d\n", seqNum);
		seqNum += req;
	}

}
