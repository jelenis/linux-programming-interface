#include "remote.h"

static void errExit(char *str) {
	perror(str);	
	exit(EXIT_FAILURE);
}

static void reaper(int sig) {
	int e = errno;

	while(waitpid(-1, NULL, WNOHANG) > 0)
		continue;
	errno = e;
}

static void serviceRequest(int cfd) {
	int masterfd;
	int childPid;
	struct termios tty;

	if (tcgetattr(STDIN_FILENO, &tty) == -1)
		errExit("tcgetattr");
	// create both a master and slave psuedoterminal
	childPid = ptyFork(&masterfd, NULL, 0, &tty, NULL);
	if (childPid == -1)
		errExit("ptyFork");

	// childPid is the slave psuedoterminal
	if (childPid == 0) { 
		tty.c_lflag &= ~ECHO;
		if (tcsetattr(STDIN_FILENO, TCSANOW, &tty) == -1) 
			errExit("tcsetattr");
		/* Execute login(1) on child pty slave */
		execlp("login", "login",  (char *) NULL);
		errExit("exec"); // should never reach here
	} 

	/* Parent falls here, this is the master psuedoterminal.
	   relay data between the master and slave here using epoll*/
	int epfd, nfds;
	ssize_t num;
	char buf[256];
	struct epoll_event ev, events[2];
	//ttySetRaw(STDIN_FILENO, &tty);
	       
	epfd = epoll_create1(0);
	if (epfd == -1)
		errExit("epoll_create1");

	/* Monitor read events on the client (cfd) and on the master
	   psuedoterminal (masterfd) */
	ev.events = EPOLLIN;
	ev.data.fd = cfd;
	if (epoll_ctl(epfd, EPOLL_CTL_ADD, cfd, &ev) == -1)
		errExit("epoll_ctl");

	ev.data.fd = masterfd;
	if (epoll_ctl(epfd, EPOLL_CTL_ADD, masterfd, &ev) == -1)
		errExit("epoll_ctl");
	for (;;) {
		/* make blocking call until there is input to from cfd or 
		   output from the slave */
		nfds = epoll_wait(epfd, events, 10, -1);	
		if (nfds == -1)
			errExit("epoll_wait");

		for (int n = 0; n < nfds; n++) {
			if (events[n].data.fd == cfd) {
				// cfd --> pty
				num = read(cfd, buf, sizeof(buf));
				if (num <= 0) // eof or error
					exit(EXIT_SUCCESS);
				buf[num] = '\0';
						
				printf("cfd --> [%s]:%d --> masterfd\n", buf, num);
				if (write(masterfd, buf, num) != num)
					errExit("partial write (masterfd)");
			} else if (events[n].data.fd == masterfd) {
				// pty --> cfd
				num = read(masterfd, buf, sizeof(buf));
				if (num <= 0) // eof or error
					exit(EXIT_SUCCESS);
				buf[num] = '\0';
				printf("masterfd --> [%s]:%d --> cfd\n", buf, num);
				//printf("%d %x \t %d %x\n", num-2, buf[num-2], num-1, buf[num-1]);
			
				if (write(cfd, buf, num) != num)
					errExit("partial write (cfd)");
			}
		}

	}	
	

}

int main() {
	struct sigaction sa;
	int lfd;

	sa.sa_handler = reaper;
	sa.sa_flags = SA_RESTART;
	sigemptyset(&sa.sa_mask);

	if (sigaction(SIGCHLD, &sa, NULL) == -1)
		errExit("sigaction");

	lfd = inetListen(SERVICE, BACKLOG, NULL);
	if (lfd == -1)
		errExit("inetListen");

	for (;;) {
		int cfd = accept(lfd, NULL, 0);
		if (cfd == -1) {
			if (errno == EINTR)
				continue;
			errExit("accept");
		}

		if (fork() == 0) {
			serviceRequest(cfd);
			exit(EXIT_SUCCESS);
		}
		close(cfd); // server doest need anymore

	}

}
