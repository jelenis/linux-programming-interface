#include <sys/epoll.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define MAX_EVENTS 10

int main() {
	struct epoll_event ev, events[MAX_EVENTS];
	int epfd, nfds;

	epfd = epoll_create1(0);
	if (epfd == -1) {
		perror("epoll_create1");
		return -1;
	}

	/* wait for epoll with empty event list will block, 
	   this may be usefull to allow another thread to add to the epoll
	   interest list */
	nfds = epoll_wait(epfd, events, MAX_EVENTS, -1);
	if (nfds == -1) {
		perror("epoll_wait");
		return -1;
	}

}
        
