#include "remote.h"
#include <fcntl.h>
#include <ctype.h>

static void errExit(char *str) {
	perror(str);
	exit(EXIT_FAILURE);
}
static char escape_buf[1000];
static char *escape(char *str) {
	char *p;
	char *e = escape_buf;
	char num[10];
	for (p = str; *p != '\0'; p++) {
		switch (*p) {
			case '\n':
				*e++ = '\\';
				*e++ = 'n';
				break;
			case '\r':
				*e++ = '\\';
				*e++ = 'r';
				break;
			
			default:
				if (isalnum(*p) || *p == ' ' || ispunct(*p)) {
					*e++ = *p;
				} else {
					sprintf(num, "{%d}", *p);
					strcat(e,num);
					e += strlen(num);
				}
		}		
	}
	*e = '\0';
	return escape_buf;
}
int main(int argc, char *argv[]) {
	int cfd, epfd, nfds, flags;
	ssize_t num;
	char host[32];
	struct epoll_event ev, ep, events[2];
	struct termios tty;

	if (tcgetattr(STDIN_FILENO, &tty) == -1)
		errExit("tcgetattr");
	
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

	ep.events = EPOLLIN;
	ep.data.fd = STDIN_FILENO;
	if (epoll_ctl(epfd, EPOLL_CTL_ADD, STDIN_FILENO, &ep) == -1)
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

				printf("reading socket\n");
				num = read(cfd, buf, sizeof(buf));
				if (num <= 0) {
					printf("connection closed\n");
					exit(EXIT_SUCCESS);
				}
				buf[num] = '\0';
				//if (strncmp(buf, "Password: ", 10) == 0) {
					//tty.c_lflag &= ~ ECHO;
					//if (tcsetattr(STDIN_FILENO, TCSANOW, &tty) == -1)
						//errExit("tcsetattr");
					//fflush(stdin);
				//}
				if (write(STDOUT_FILENO, buf, num) != num)
					errExit("partial write (stdout)");
				fflush(stdout);

			}
			if (events[n].data.fd == STDIN_FILENO) {
				
				printf("reading stdin\n");
				num = read(STDIN_FILENO, buf, sizeof(buf));
				if (num <= 0) {
					printf("connection closed\n");
					exit(EXIT_SUCCESS);
				}
				buf[num] = '\0';

				//if (!(tty.c_lflag & ECHO)) {
					//tty.c_lflag |= ECHO;
					//if (tcsetattr(STDIN_FILENO, TCSANOW, &tty) == -1)
						//errExit("tcsetattr");
					//printf("\n");
				//}


				if (write(cfd, buf, num) != num)
					errExit("partial write (stdout)");
			}

		}

	}

}
