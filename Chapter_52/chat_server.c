/**
 * A simple talk(1) like messaging service.
 * Uses unidirectional POSIX message queues to route message to a from clients.
 * A client sends a message to a user by placing the message on the server queue,
 * the server then maps forwards the message to the correct POSIX queue of the 
 * recipient.
 *
 * Uses Multiprocessing for simplicity, multithreading would make more sense.
 */
#include "chat.h"
#include <errno.h>

/**
 * Kill off zombie children
 */
static void handler(int signum) {
	int e = errno;

	while (waitpid(-1, NULL, WNOHANG) > -1)
		continue;
	errno = e;
}

/**
 * Handle requests concurrently
 */
static void handleRequest(Message *msg) {
	struct mq_attr attr;
	char cq_name[NAME_MAX] = "/";	
	mqd_t cq;
	strcat(cq_name, msg->to);

	umask(0);
	attr.mq_msgsize = sizeof(Message);
	attr.mq_maxmsg = 10;
	cq = mq_open(cq_name, O_CREAT | O_WRONLY, perms, &attr);
	if (cq == -1) {
		perror("server: open client");
		_exit(EXIT_FAILURE);
	}

	printf("sent %s to %s\n", msg->data, cq_name);
	if (mq_send(cq, (char*)msg, attr.mq_msgsize, 0) == -1) {
		perror("server send");
		_exit(EXIT_FAILURE);
	}
}

/**
 * Recieves messages from the SRV POSIX message queue and forwards them
 * the appropriate user queue that is analogous to their username
 */
int main() {
	struct sigaction sa;
	struct mq_attr attr;
	mqd_t mqd;
	
	// install a signal handler to reap zombie children
	sa.sa_handler = handler;
	sa.sa_flags = 0; // SA_RESTART doesnt work on message queues
	if (sigaction(SIGCHLD, &sa, NULL) == -1) {
		perror("server: sigaction");
		exit(EXIT_FAILURE);
	}

	attr.mq_msgsize = sizeof(Message);
	attr.mq_maxmsg = 10;
	umask(0); // needed so that other users can open the message queue
	mqd = mq_open(SERVER, O_CREAT | O_RDONLY, perms, &attr);
	if (mqd == -1) {
		perror("server: open");
		exit(EXIT_FAILURE);
	}

	/* loop forever waiting for clients to send messages */
	for(;;) {
		Message msg;
		ssize_t numBytes = mq_receive(mqd, (char*)&msg, attr.mq_msgsize, NULL); 
		if (numBytes == -1) {
			// ignore signal interuptions
			if (errno == EINTR)
				continue;
			perror("server: receive");
			exit(EXIT_FAILURE);
		}

		if (fork() == 0) {
			handleRequest(&msg);	
			_exit(EXIT_SUCCESS);
		}

	}
}
