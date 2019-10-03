#define _GNU_SOURCE
#include "remote.h"
#include <utmpx.h>
#include <paths.h>
#include <time.h>

int childPid;

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

void master_exit() {
	struct utmpx ut;

	kill(childPid, SIGKILL);
	ut.ut_type = DEAD_PROCESS;
	time((time_t *) &ut.ut_tv.tv_sec);
	memset(ut.ut_user, 0, sizeof(ut.ut_user));

	setutxent();
	if (pututxline(&ut) == NULL)
		errExit("pututxline");

	updwtmpx(_PATH_WTMP, &ut);
	endutxent();

	exit(EXIT_SUCCESS);
}


static void serviceRequest(int cfd) {
	int masterfd;
	struct termios tty;
	char slaveName[MAX_NAME];

	if (tcgetattr(STDIN_FILENO, &tty) == -1)
		errExit("tcgetattr");
	// create both a master and slave pseudoterminal
	childPid = ptyFork(&masterfd, slaveName, MAX_NAME, &tty, NULL);
	if (childPid == -1)
		errExit("ptyFork");
	if (strlen(slaveName) <= 8)
		errExit("terminal name too short");

	// childPid is the slave pseudoterminal
	if (childPid == 0) { 
		/* remove echooing (else this will come back and print what
		   the user had already typed) */
		tty.c_lflag &= ~ECHO;
		if (tcsetattr(STDIN_FILENO, TCSANOW, &tty) == -1) 
			errExit("tcsetattr");
		/* Execute login(1) on child pty slave */
		execlp("login", "login",  (char *) NULL);
		errExit("exec"); // should never reach here
	} 
	
	if (atexit(master_exit) != 0)
		errExit("atexit");

	/* Parent falls here, this is the master pseudoterminal.
	   relay data between the master and slave here using epoll*/
	int epfd, nfds;
	ssize_t num;
	char buf[256];
	int logged_in = 0, first = 1; // bool
	struct epoll_event ev, events[2];
	struct utmpx ut;


	memset(&ut, 0, sizeof(struct utmpx));
	ut.ut_pid = childPid;
	ut.ut_type = USER_PROCESS;
	// use the pseudo termial name as for utmpx timestamp
	strncpy(ut.ut_line, slaveName + 5, sizeof(ut.ut_line));
	strncpy(ut.ut_id, slaveName + 8, sizeof(ut.ut_id));
	       
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

				if (!logged_in && first) {
					// get the user name 
					strncpy(ut.ut_user, buf, num - 1);
					first = 0;
				}
						
				if (write(masterfd, buf, num) != num)
					errExit("partial write (masterfd)");
			} else if (events[n].data.fd == masterfd) {
				// pty --> cfd
				num = read(masterfd, buf, sizeof(buf));
				if (num <= 0) // eof or error
					exit(EXIT_SUCCESS);
				buf[num] = '\0';

				// time stamp in wtmp and utmpx file
				if (!logged_in && strncmp(buf, "Last login:", 11) == 0) {
					logged_in = 1;

					printf("Session started: %s %s %s\n", ut.ut_user, ut.ut_line, ut.ut_id);
					if (time((time_t *) &ut.ut_tv.tv_sec) == -1)
						errExit("time");

					setutxent(); // re-wind (good practice)
					if (pututxline(&ut) == NULL)
						errExit("pututxline");
					updwtmpx(_PATH_WTMP, &ut);
				} else if (!logged_in && strcmp(buf, "Login incorrect") == 0) {
					first = 1; // attempt login again
				}
			
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
