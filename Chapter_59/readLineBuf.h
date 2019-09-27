/* 
 * A simple buffering mechanism to read from a file descriptor with minimal writes
 */
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>

#define BUF_SIZE 1024
struct rl {
	int fd;
	size_t c;	
	size_t size;
	char buf[BUF_SIZE];
};

/**
 * Initializes the rlbuf structure for bookeeping.
 */
void readLineBufInit(int fd, struct rl *rlbuf); 

/**
 * Provide a buffering mechanism to limit the number of read sysytem calls
 * and fill buffer with up to n bytes read from fd in rlbuf.
 * Returns the number of bytes places in buffer
 */
ssize_t readLineBuf(struct rl *rlbuf, char* buffer, size_t n);
