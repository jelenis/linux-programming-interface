/**
 * Echo server that runs accepts both tcp and udp using the epoll API
 */
#include <sys/socket.h>
#include <stdio.h>
#include <sys/epoll.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include "inet_sockets.h"
#include <netdb.h>

#define BACK_LOG 10
#define PORT "50000"
static void errExit(char *str) {
	perror(str);
	exit(EXIT_FAILURE);
}

int main() {
	int epfd, lfd_tcp, lfd_udp;	
	struct epoll_event ev[2], events[BACK_LOG];
	epfd = epoll_create1(0);

	if (epfd == -1)
		errExit("epoll_creat1");

	lfd_tcp = inetListen(PORT, BACK_LOG, NULL);
	if (lfd_tcp == -1)
		errExit("inetListen");

	lfd_udp = inetBind(PORT, SOCK_DGRAM, NULL);
	if (lfd_udp == -1)
		errExit("inetBind");

	/* Create an event for each socket */
	ev[0].events = EPOLLIN;
	ev[0].data.fd = lfd_tcp;
	if (epoll_ctl(epfd, EPOLL_CTL_ADD, lfd_tcp, &ev[0]) == -1)
		errExit("epoll_ctl tcp");

	ev[1].events = EPOLLIN;
	ev[1].data.fd = lfd_udp;
	if (epoll_ctl(epfd, EPOLL_CTL_ADD, lfd_udp, &ev[1]) == -1)
		errExit("epoll_ctl udp");

	for (;;) {
		char buf[256];
		ssize_t num;
		int cfd;
		// fully qulified socket connection 

		// block until a tcp or udp connection (returns num of events)
		int nfds = epoll_wait(epfd, events, BACK_LOG, -1);
		if (nfds == -1)
			errExit("epoll_wait");

		for (int i = 0; i < nfds; i++) {
			// TCP connection
			if (events[i].data.fd == lfd_tcp) {
				int cfd = accept(lfd_tcp, NULL, NULL); 
				if (cfd == -1)
					errExit("accept");

				while ((num = read(cfd, buf, sizeof(buf))) > 0)
					if (write(cfd, buf, num) != num)
						errExit("partial write tcp");

			} else { // UDP connection
				struct sockaddr_storage claddr;
				socklen_t len = sizeof(struct sockaddr_storage);
				num = recvfrom(lfd_udp, buf, sizeof(buf), 0,
					       	(struct sockaddr *) &claddr, &len);
				if (num == -1)
					errExit("recvfrom");

				num = sendto(lfd_udp, buf, num, 0,
					       	(struct sockaddr *) &claddr, len);
				if (num == -1)
					errExit("sendto");
			}
		}



	}

}



