#include "unix_sockets.h"


int main() {
	int sfd;

        sfd = unListen(SV_SOCKET, 10);
	if (sfd == -1) {
		perror("unListen");
		return -1;
	}

	for (;;) {
		char buf[BUF_SIZE];
		int cfd;
		ssize_t numRead;
	       
		cfd = accept(sfd, NULL, NULL);
		if (cfd == -1) {
			perror("accept");
			continue;
		}

		while ((numRead = read(cfd, buf, sizeof(buf))) > 0) {
			write(STDOUT_FILENO, buf, numRead);
		}

		if (close(cfd) == -1) {
			perror("close");
			return -1;
		}

	}

}
