#include "server.h"

static void reaper(int signum) {
	int e = errno; // prevent errno from being altered 
	// reap all children 
	while(waitpid(-1, NULL, WNOHANG) > 0)
		continue;
	errno = e;
}

/**
 * Recieves requests sent to sender and responds with
 * the start of the clients sequence
 */
static void serveRequest(Message* req, int seqNum){
	Message resp;	
	resp.mtype = req->from; // send back to requester
	resp.from = 1;
	resp.seq = seqNum;
	
	// cannot do much about this error
	if (msgsnd(KEY, &resp, MSG_SIZE, 0) == -1)
		perror("msgsnd");
}

int main () {
	int msqid;
	int seqNum = 0;
	struct sigaction sa;
	
	/* message queue with read and write permissions for
	   bidirectional communication */
	msqid = msgget(KEY, IPC_CREAT | IPC_EXCL 
			| S_IRUSR | S_IWUSR | S_IWGRP | S_IRGRP);
	if (msqid == -1) {
		perror("msgget");
		exit(EXIT_FAILURE);
	}

	// handler for terminated children
	sa.sa_handler = reaper;
	sa.sa_flags = SA_RESTART;
	sigemptyset(&sa.sa_mask);
	if (sigaction(SIGCHLD, &sa, NULL) == -1) {
		perror("sigaction");
		exit(EXIT_FAILURE);		
	}

	/* read incoming messages with the addressed to this process(type = 1)
	   and handle the requests in a seperate process */
	for(;;) {
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
		// allocate sequence for this request
		seqNum += req.seq;

	}


}

