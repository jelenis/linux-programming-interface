#include "seqnum.h"

/**
 * Server sequece number service. Allocates a sequence of numbers for each client.
 * Requests are made through a unidirectionalPOSIX message queue and responses are 
 * sent to each process individually through an ad-hoc message queue identified by
 * the pid of the client.
 */
int main() {
	int seqNum = 0;
	Request req;
	struct mq_attr attr;
	mqd_t mqd, cq;
	
	attr.mq_maxmsg = 10;
	attr.mq_msgsize = sizeof(Request);
	
	// open a posix message queue to receieve client requests
	mqd = mq_open(SERVER, O_RDWR | O_CREAT,
		       	S_IRUSR | S_IWUSR, &attr);
	if (mqd == (mqd_t)-1) {
		perror("mq_open");
		exit(EXIT_FAILURE);	
	}

	for (;;) {
		ssize_t numBytes = mq_receive(mqd, (void*)&req, attr.mq_msgsize, NULL);

		if (numBytes == -1) {
			perror("server: receive");
			exit(EXIT_FAILURE);
		}
		
		// open clients queue
		cq = mq_open(req.cq, O_WRONLY);
		if (cq == -1) {
			perror("server: open");
			exit(EXIT_FAILURE);
		}

		/* respond to each request with the starting value of their
		   allocated sequence */
		if (mq_send(cq, (char*)&seqNum, sizeof(int), 0) == -1) {
			perror("server: send");
			exit(EXIT_FAILURE);
		}
		seqNum += req.seq;


	}
}
