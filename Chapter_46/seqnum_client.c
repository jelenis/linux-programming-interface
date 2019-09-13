#include "server.h"
#include <string.h>

/**
 * Requests a sequence of numbers from the server
 * using the length specified in argv[2]
 */
int main (int argc, char* argv[]) {
	Message req, resp;
	int msqid, pid, fd;
	int seq_len = 0;

	if (argc < 2) {
		printf("Enter the length of the sequence\n");
		exit(EXIT_FAILURE);
	}

	seq_len = strtod(argv[1], NULL);
	if (seq_len <= 0) {
		printf("Length must be greater than zero\n");
		exit(EXIT_FAILURE);
	}
	
	// load identifier from common file location
	fd = open(KEY_PATH, O_RDONLY, S_IRUSR);
	if (fd == -1) {
		perror("open");
		exit(EXIT_FAILURE);
	}
	
	if (read(fd, &msqid, sizeof(msqid)) != sizeof(msqid)) {
		perror("read");
		exit(EXIT_FAILURE);
	}
	close(fd);

	// create request
	pid = getpid();
	req.mtype = 1; //server addr	
	req.from = pid;
	req.seq = seq_len;
	
	// send a message to the server
	if (msgsnd(msqid, &req, MSG_SIZE, 0) == -1) {
		perror("msgsnd");
		exit(EXIT_FAILURE);
	}
	sleep(20);
	// block and wait for a response that has the address of this pid
	if (msgrcv(msqid, &resp, MSG_SIZE, pid, 0) == -1) {
		perror("msgrcv");
		exit(EXIT_FAILURE);
	}

	printf("Received sequence: %d\n", resp.seq);

}
