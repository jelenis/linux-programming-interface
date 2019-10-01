#include <stdio.h>
#include <sys/select.h>
#include <stdlib.h>
#include <sys/msg.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <errno.h>

static void errExit(char *str) {
	perror(str);
	exit(EXIT_FAILURE);
}

struct mbuf {
	long mtype;
	char mtext[1024];
};

int main(int argc, char *argv[]) {
	int nfds = STDIN_FILENO, flags;	
	int p[2];
	fd_set set;
	char buf[1024];
	struct mbuf msg;
	key_t key = ftok(argv[1], 1);
	int msqid = msgget(key, IPC_CREAT);

	if (argc < 2) {
		fprintf(stderr, "missing file for key\n");
		exit(EXIT_FAILURE);
	}
	
	FD_ZERO(&set);
	FD_SET(STDIN_FILENO, &set);

	if (pipe(p) == -1)
		errExit("pipe");
	
	// make read end non-block
	flags = fcntl(p[0], F_GETFL);
	if (flags == -1)
		errExit("F_GETFL");
	flags |= O_NONBLOCK;
	if (fcntl(p[0], F_SETFL, flags) == -1)
		errExit("F_SETFL");

	// make write end non-block
	flags = fcntl(p[1], F_GETFL);
	if (flags == -1)
		errExit("F_GETFL");
	flags |= O_NONBLOCK;
	if (fcntl(p[1], F_SETFL, flags) == -1)
		errExit("F_SETFL");

	if (p[0] > nfds)
		nfds = p[0] + 1;
	if (p[1] > nfds)
		nfds = p[1] + 1;

	// monitor read end
	FD_SET(p[0], &set);

	if (fork() == 0) {
		// close uneeded read end
		close(p[0]); 
		for (;;) {
			ssize_t num = msgrcv(msqid, &msg, sizeof(struct mbuf), 0, 0);
			if (num == -1)
				errExit("msgrcv");
			if (write(p[1], msg.mtext, num) != num)
				errExit("write");

		}
		// should never get here
		_exit(EXIT_FAILURE);
	}
	// close write end
	close(p[1]);
	ssize_t num; 
	while (1) {
		int res = select(nfds, &set, NULL, NULL, NULL);
		if (res == -1)
			errExit("select");	
		if (FD_ISSET(p[0], &set)) {
			// message queue event
			num = read(p[0], buf, sizeof(buf));
			if (num == -1)
				errExit("read");
			buf[num] = '\0';
			printf("Message recieved: %s\n", buf);
		} else { 
			// stdin read event
			num = read(STDIN_FILENO, buf, sizeof(buf));	
			printf("recieved %d bytes\n", num);
		}
	}


}

