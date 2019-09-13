#include "server.h"
#include <syslog.h>
static int msqid;

/* timeout used to catch blocking system calls*/
static void timeout(int signum) {
	printf("timedout\n");
	syslog(LOG_DAEMON | LOG_ERR, "Dropped client");
}

/**
 * Wait for zombie process that have finished serving clients
 */
static void reaper(int signum) {
	int e = errno; // prevent errno from being altered 
	// reap all children 
	while(waitpid(-1, NULL, WNOHANG) > 0)
		continue;
	errno = e;
}

static void clean_up (int signum) {
	if (msgctl(msqid, IPC_RMID, NULL) == -1) {
		syslog(LOG_DAEMON | LOG_ERR, "Could not remove queue %d", msqid);
		exit(EXIT_FAILURE);
	}
		
	/* establish default handler and raise the same signal so 
	   the calling process can see status  */
	signal(signum, SIG_DFL);
	syslog(LOG_DAEMON | LOG_NOTICE, "Sequence service is shutting down...");
	if (raise(signum) != 0)	
		exit(EXIT_FAILURE);
}

/**
 * Recieves requests sent to sender and responds with
 * the start of the clients sequence.
 * will timeout if msgsnd blocks as a result of a full msg queue
 */
static void serveRequest(Message* req, int seqNum){
	struct sigaction sa;	
	Message resp;	

	/* handler for alarm timemout, must be declared in this process 
	   as alarms are cleared over fork */
	sa.sa_handler = timeout;
	sa.sa_flags = 0;
	if (sigaction(SIGALRM, &sa, NULL) == -1) {
		perror("sigaction");
		exit(EXIT_FAILURE);		
	}

	resp.mtype = req->from; // send back to requester
	resp.from = 1;
	resp.seq = seqNum;
	
	// set an alarm for timeout
	if (alarm(2) != 0) {
		syslog(LOG_DAEMON | LOG_ERR, "could not set timeout for request %d", req->from);
		exit(EXIT_FAILURE);
	}
	// cannot do much about this error
	if (msgsnd(msqid, &resp, MSG_SIZE, 0) == -1) {
		perror("msgsnd");
		syslog(LOG_DAEMON | LOG_ERR, "could not write message to %d", req->from);
	}

	alarm(0); // cancel alarm
	
}

int main () {
	int fd, seqNum = 0;
	struct sigaction sa;
	
	/* message queue with read and write permissions for
	   bidirectional communication */
	msqid = msgget(IPC_PRIVATE, IPC_CREAT | IPC_EXCL 
			| S_IRUSR | S_IWUSR | S_IWGRP | S_IRGRP);
	if (msqid == -1) {
		perror("msgget");
		exit(EXIT_FAILURE);
	}
	
	/* write identifier to a common file location */
	fd = open(KEY_PATH, O_CREAT | O_WRONLY,
				S_IRUSR | S_IWUSR | S_IWGRP | S_IRGRP);
	if (fd == -1) {
		perror("open");
		exit(EXIT_FAILURE);
	}
	if (write(fd, &msqid, sizeof(msqid)) != sizeof(msqid)) {
		perror("write");
		exit(EXIT_FAILURE);
	}
	if (close(fd) == -1){
		perror("close");
		exit(EXIT_FAILURE);
	}

	// handler for SIGINT and SIGTERM
	sa.sa_handler = clean_up;
	sa.sa_flags = 0;
	if (sigaction(SIGINT, &sa, NULL) == -1) {
		perror("sigaction");
		exit(EXIT_FAILURE);		
	}
	if (sigaction(SIGTERM, &sa, NULL) == -1) {
		perror("sigaction");
		exit(EXIT_FAILURE);		
	}

	// handler for terminated children
	sa.sa_handler = reaper;
	sa.sa_flags = SA_RESTART;
	if (sigaction(SIGCHLD, &sa, NULL) == -1) {
		perror("sigaction");
		exit(EXIT_FAILURE);		
	}
	

	/* read incoming messages with the addressed to this process(type = 1)
	   and handle the requests in a seperate process */
	for (;;) {
		Message req;	
		int pid;
		// block until a message with type 1 is received
		int len = msgrcv(msqid, &req, MSG_SIZE, 1, 0);
		/* restart loop if error is a result of a signal interruption
		   since it is impossible to restart these calls using SA_RESART */
		if (len == -1) {
			if (errno == EINTR)
				continue; // try again
			perror("msgrcv");
			break; // else shutdown server
		}

		/* Create new process to handle request */
		pid = fork();
		if (pid == -1) {
			perror("fork");
			break; // loop again
		} else if (pid == 0) {
			// allocate a sequence to this process
			serveRequest(&req, seqNum);
			_exit(EXIT_SUCCESS);
		}
		// move allocated sequence up
		seqNum += req.seq;
	}

}

