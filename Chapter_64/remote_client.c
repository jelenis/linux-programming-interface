#include "remote.h"
static void errExit(char *str) {
	perror(str);
	exit(EXIT_FAILURE);
}

int main(int argc, char *argv[]) {
	int cfd, epfd, nfds;
	ssize_t num;
	char host[32];
	struct epoll_event ev, events[2];

	if (argc < 2) {
		strcpy(host, "localhost");
	}		


	cfd = inetConnect(host, SERVICE, SOCK_STREAM);
	if (cfd == -1)
		errExit("inetConnect");

	epfd = epoll_create1(0);
	if (epfd == -1)
		errExit("epoll_create1");

	ev.events = EPOLLIN;
	ev.data.fd = cfd;
	if (epoll_ctl(epfd, EPOLL_CTL_ADD, cfd, &ev) == -1)
		errExit("epoll_ctl");
	ev.data.fd = STDIN_FILENO;
	if (epoll_ctl(epfd, EPOLL_CTL_ADD, STDIN_FILENO, &ev) == -1)
		errExit("epoll_ctl");

	for (;;) {
		char buf[256];
		// block and wait for events
		nfds = epoll_wait(epfd, events, 2, -1);
		if (nfds == -1) {
			if (errno == EINTR)
				continue;
			errExit("epoll_wait");
		}

		for (int n = 0; n < 2; n++) {
			if (events[n].data.fd == cfd) {
				num = read(cfd, buf, sizeof(buf));
				if (num <= 0) {
					printf("connection closed\n");
					exit(EXIT_SUCCESS);
				}
				if (write(STDOUT_FILENO, buf, num) != num)
					errExit("partial write (stdout)");
				fflush(stdout);
				
			} else if (events[n].data.fd == STDIN_FILENO) {
				num = read(STDIN_FILENO, buf, sizeof(buf));
				if (num <= 0) {
					printf("connection closed\n");
					exit(EXIT_SUCCESS);
				}
				if (write(cfd, buf, num) != num)
					errExit("partial write (stdout)");
			}

		}

	}

}
