/* 
 * A simple buffering mechanism to read from a file descriptor with minimal writes
 */
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>

#define BUF_SIZE 10
static char result[BUF_SIZE];
struct rl {
	int fd;
	char * c;	
	size_t size;
	char buf[BUF_SIZE];
};

/**
 * Initializes the rlbuf structure for bookeeping.
 */
void readLineBufInit(int fd, struct rl *rlbuf) {
	rlbuf->fd = fd;
	rlbuf->size = BUF_SIZE;		
	// point to one past the last character of buffer
	rlbuf->c = rlbuf->buf + rlbuf->size;
}

/**
 * Provide a buffering mechanism to limit the number of read sysytem calls
 * and fill buffer with up to n bytes read from fd in rlbuf.
 * Returns the number of bytes places in buffer
 */
ssize_t readLineBuf(struct rl *rlbuf, char* buffer, size_t n) {
	ssize_t numRead;
	ssize_t cnt = 0;
	if (rlbuf == NULL) {
		errno = EINVAL;
		return -1;
	}

	/* if the next character is the end of buffer read from fd 
	   else just read from the buffer */
	if (rlbuf->c == rlbuf->buf + rlbuf->size) {
		rlbuf->c = rlbuf->buf; // reset to beginning of buffer 
		// read a chunck of data and scan for newline
		numRead = read(rlbuf->fd, rlbuf->buf, rlbuf->size);
		if (numRead == -1)
			return -1;
		//printf("\t\tread %s\n", rlbuf->buf);
	}

	for (; rlbuf->c < (char*)rlbuf->buf + rlbuf->size; rlbuf->c++) {
		if (*rlbuf->c == '\n')
			break;
		if (cnt < n)	
			buffer[cnt++] = *rlbuf->c;
	}
	buffer[cnt] = '\0';

	return cnt;
}

