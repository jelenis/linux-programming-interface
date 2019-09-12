#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <wait.h>
#include <errno.h>

static void removeFifo() {
	unlink("fifo");
}

/**
 * Demonstrates the use of O_NONBLOCK in fifos.
 * You can only open a nonblocking fifo for writing if there is a corresponding open
 * reading end. Though you may open a nonblocking reading end without an open writing end.
 */
int main(int argc, char* argv[]) {
	if (mkfifo("fifo", S_IRUSR | S_IWUSR | S_IWGRP) == -1) {
		perror("mkfifo");
		exit(EXIT_FAILURE);
	}
	if (atexit(removeFifo) != 0) {
		perror("atexit");
		exit(EXIT_FAILURE);
	}

	// open fifo non blocking
	int wd = open("fifo", O_WRONLY | O_NONBLOCK);
	if (wd == -1) {
		perror("write open");
	} else
		printf("fifo is opened for writing\n");

	int rd = open("fifo", O_RDONLY | O_NONBLOCK);
	if (rd == -1) {
		perror("read open");
	} else
		printf("fifo is opened for reading\n");

	if (write(wd, "1", 1) != 1)
		perror("write to fifo");
	else
		printf("wrote 1 byte to fifo\n");

	ssize_t nbytes = 0;
	char buf[1];
	errno = 0;
	if ((nbytes = read(rd, buf, 1)) != 1) {
		printf("%s read %d bytes from fifo\n", strerror(errno), nbytes);
	} else {
		printf("read 1 byte from fifo\n");
	}

	wd = open("fifo", O_WRONLY | O_NONBLOCK);
	if (wd == -1) {
		perror("write open");
	} else {
		printf("fifo is opened for writing\n");
	}

	if (write(wd, "1", 1) != 1)
		perror("write to fifo");
	else
		printf("wrote 1 byte to fifo\n");

	errno = 0;
	if ((nbytes = read(rd, buf, 1)) != 1) {
		printf("%s read %d bytes from fifo\n", strerror(errno), nbytes);
	} else {
		printf("read 1 byte from fifo\n");
	}
}
