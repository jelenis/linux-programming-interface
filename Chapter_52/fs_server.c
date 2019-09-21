#include "fs.h"

static void handleRequest(Request *req) {
	Response resp;
	struct mq_attr cq_attr;
	int fd;
	ssize_t bytesRead;

	// open clients queue
	mqd_t cq = mq_open(req->cq, O_WRONLY);
	if (cq == -1) {
		perror("server: open");
		_exit(EXIT_FAILURE);
	}
	printf("servicing reqeust: {%s %s}\n", req->cq, req->file);
	
	fd = open(req->file, O_RDONLY);
	if (fd == -1) {
		perror("open");
		_exit(EXIT_FAILURE);
	}

	if (mq_getattr(cq, &cq_attr) == -1) {
		perror("server: getattr");
		_exit(EXIT_FAILURE);
	}

	/* send the file to the client in their dedicated queue*/
	resp.type = DATA;
	while ((bytesRead = read(fd, resp.buf, sizeof(resp.buf))) > 0) {

		if (mq_send(cq, (char*)&resp, cq_attr.mq_msgsize, 0) == -1) {
			perror("server: send");
			_exit(EXIT_FAILURE);
		}
	}
	resp.type = END;
	memset(resp.buf, 0,  sizeof(resp.buf));
	if (mq_send(cq, (char*)&resp, cq_attr.mq_msgsize, 0) == -1) {
		perror("server: send");
		_exit(EXIT_FAILURE);
	}
}

/**
 * Serivices reqeusts for a local file from a client.
 */
int main() {
	Request req;
	struct mq_attr attr;
	mqd_t mqd;
	
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
		ssize_t numBytes = mq_receive(mqd, (char*)&req, attr.mq_msgsize, NULL);
		int pid;

		if (numBytes == -1) {
			perror("server: receive");
			exit(EXIT_FAILURE);
		}
		
		if (fork() == 0) {
			handleRequest(&req);
			exit(EXIT_SUCCESS);
		} 

		
	}
}
