/**
 * Removes stdio buffering on program execed in argv[1] 
 */
#define _GNU_SOURCE
#include "remote.h"
#include <utmpx.h>
#include <paths.h>
#include <time.h>
#include <ctype.h>

struct termios tty;

static void errExit(char *str) {
	perror(str);	
	exit(EXIT_FAILURE);
}

void master_exit() {
	if (tcsetattr(STDIN_FILENO, TCSANOW, &tty) == -1)
		errExit("tcgetattr");

	printf("\r\nConnection Closed");
	exit(EXIT_SUCCESS);
}

static void serviceRequest(char *argv[]) {
	int masterfd, childPid;
	char slaveName[MAX_NAME];

	if (tcgetattr(STDIN_FILENO, &tty) == -1)
		errExit("tcgetattr");
	ttySetRaw(STDIN_FILENO, &tty);

	// create both a master and slave pseudoterminal
	childPid = ptyFork(&masterfd, slaveName, MAX_NAME, &tty, NULL);
	if (childPid == -1)
		errExit("ptyFork");
	if (strlen(slaveName) <= 8)
		errExit("terminal name too short");

	// childPid is the slave pseudoterminal
	if (childPid == 0) { 
		/* Execute program on child pty slave */
		execvp(argv[1], &argv[1]);
		errExit("exec"); // should never reach here
	} 
	
	if (atexit(master_exit) != 0)
		errExit("atexit");

	/* Parent falls here, this is the master pseudoterminal.
	   relay data between the master and slave here using epoll*/
	int epfd, nfds;
	ssize_t num;
	char buf[256];
	struct epoll_event ev, events[2];

	epfd = epoll_create1(0);
	if (epfd == -1)
		errExit("epoll_create1");

	/* Monitor read events stdin and on the master
	   psuedoterminal (masterfd) */
	ev.events = EPOLLIN;
	ev.data.fd = STDIN_FILENO;
	if (epoll_ctl(epfd, EPOLL_CTL_ADD, STDIN_FILENO, &ev) == -1)
		errExit("epoll_ctl");

	ev.data.fd = masterfd;
	if (epoll_ctl(epfd, EPOLL_CTL_ADD, masterfd, &ev) == -1)
		errExit("epoll_ctl");
	
	for (;;) {
		/* make blocking call until there is input to from cfd or 
		   output from the slave */
		nfds = epoll_wait(epfd, events, 2, -1);	
		if (nfds == -1)
			errExit("epoll_wait");

		for (int n = 0; n < nfds; n++) {
			if (events[n].data.fd == STDIN_FILENO) {
				// cfd --> pty
				num = read(STDIN_FILENO, buf, sizeof(buf));
				if (num <= 0) // eof or error
					exit(EXIT_SUCCESS);
				buf[num] = '\0';
						
				if (write(masterfd, buf, num) != num)
					errExit("partial write (masterfd)");

			} else if (events[n].data.fd == masterfd) {
				// pty --> stout
				num = read(masterfd, buf, sizeof(buf));
				if (num <= 0) // eof or error
					exit(EXIT_SUCCESS);
				buf[num] = '\0';

				if (write(STDOUT_FILENO, buf, num) != num)
					errExit("partial write (cfd)");
			}
		}

	}	
	

}

int main(int argc, char *argv[]) {
	int lfd;

	if (argc < 2) {
		fprintf(stderr, "missing program\n");
		exit(EXIT_FAILURE);
	}


	serviceRequest(argv);

}
