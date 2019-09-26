#include "unix_sockets.h"

int main() {
	int sfd;
	ssize_t numRead;
	char buf[BUF_SIZE];

	sfd = unConnect(SV_SOCKET, SOCK_STREAM);
	if (sfd == -1) {
		perror("unConnect");
		return -1;
	}

	while ((numRead = read(STDIN_FILENO, buf, sizeof(buf))) > 0)
		if (write(sfd, buf, numRead) == -1) {
			perror("write");
			return -1;	
		}

	if (numRead == -1) {
		perror("read");
		return -1;
	}

}
