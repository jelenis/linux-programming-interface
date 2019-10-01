#include <sys/epoll.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define MAX_EVENTS 1

/**
 * Demonstrates how the order of events returned by epoll_wait
 * will prevent starvation by using round robin
 */
int main() {
	struct epoll_event ev, events[MAX_EVENTS];
	int epfd, nfds;

	epfd = epoll_create1(0);
	if (epfd == -1) {
		perror("epoll_create1");
		return -1;
	}

	for (int i = 0; i < 10; i++) {
		int p[2];
		if (pipe(p) == -1) {
			perror("pipe");
			return -1;
		}

		ev.events = EPOLLOUT;
		ev.data.fd = p[1];
		if (epoll_ctl(epfd, EPOLL_CTL_ADD, p[1], &ev) == -1) {
			perror("epoll_ctl");
			return -1;
		}
	}

	/* Loop through each event since we can only read 1 at a time
	   the order is automatically round robin  */
	for (;;) {
		nfds = epoll_wait(epfd, events, MAX_EVENTS, -1);
		if (nfds == -1) {
			perror("epoll_wait");
			return -1;
		}
		printf("fd: %d\n", events[0].data.fd);
		sleep(1); // wait so user can read output
	}

}
        
