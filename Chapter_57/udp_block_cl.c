#include "un.h"

/**
 * Sends alot of datagrams to client quickly and blocks after 
 * buffer is full.
 */
int main(int argc, char *argv[]) {
	int sfd;
	struct sockaddr_un svaddr;

	if (argc < 2) {
		fprintf(stderr, "Missing number of messages\n");
		return -1;
	}

	/* Create client socket */
	sfd = socket(AF_UNIX, SOCK_DGRAM, 0);
	if (sfd == -1) {
		perror("socket");
		return -1;
	}

	/* Populate server address */
	memset(&svaddr, 0, sizeof(struct sockaddr_un));
	svaddr.sun_family = AF_UNIX;
	// use abstract address with prepended null byte (requires strlen bytes)
	strncpy(&svaddr.sun_path[1], SV_SOCK, strlen(SV_SOCK));

	for (int i = 0; i < strtod(argv[1], NULL); i ++) {
		char buf[BUF_SIZE];
		sprintf(buf, "%d", i);
		if (sendto(sfd, buf, BUF_SIZE, 0, (struct sockaddr*) &svaddr, sizeof(struct sockaddr_un)) == -1) {
			perror("send");
			return -1;
		}
		printf("Sent %s to server\n", buf);
	}

	printf("Client finished\n");
}
