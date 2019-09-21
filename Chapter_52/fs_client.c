#include "fs.h"
#include <errno.h>

/**
 * Client for sequence number server. Requests the server for a sequence of numbers
 * of length specified in argv. 
 * Refer to seqnum_server.c for more details
 */
int main(int argc, char *argv[]) {
	Response resp;
	struct mq_attr mq_attr, cq_attr;
	mqd_t mqd, cq;
	Request req;
	ssize_t numBytes;

	if (argc < 2) {
		printf("Please enter a filename\n");
		exit(EXIT_FAILURE);
	}

	mqd = mq_open(SERVER, O_WRONLY);
	if (mqd == (mqd_t)-1) {
		perror("client: open");
		exit(EXIT_FAILURE);	
	}

	if (mq_getattr(mqd, &mq_attr) == -1) {
		perror("client: getattr");
		exit(EXIT_FAILURE);
	}
	cq_attr.mq_msgsize = sizeof(Response);
	cq_attr.mq_maxmsg = 10;

	/* create a dedicated posix message queue using this pid as
	   as the queues name */	
	sprintf(req.cq, "/%ld", getpid());
	strcpy(req.file, argv[1]);

	cq = mq_open(req.cq, O_CREAT | O_RDONLY | O_EXCL, S_IRUSR | S_IWUSR, &cq_attr);
	if (cq == (mqd_t)-1) {
		perror("client: open");
		exit(EXIT_FAILURE);
	}
	
	if (mq_send(mqd, (void*)&req, mq_attr.mq_msgsize, 0) == -1) {
		perror("client: send");
		exit(EXIT_FAILURE);
	}

	while ((numBytes = mq_receive(cq, (char*)&resp, cq_attr.mq_msgsize, NULL)) > 0) {
		if (resp.type == END)
			break;
		if (write(STDOUT_FILENO, resp.buf, numBytes) == -1)
			break;
	}
	if (numBytes == -1) {
		perror("client: receive");
		exit(EXIT_FAILURE);
	}

	// delete the dedicated clients queue
	if (mq_unlink(req.cq) == -1) {
		perror("mq_unlink");
		exit(EXIT_FAILURE);
	}	

}
