/* 
 * A simple buffering mechanism to read from a file descriptor with minimal writes
 */
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include "readLineBuf.h"


/**
 * Initializes the rlbuf structure for bookeeping.
 */
void readLineBufInit(int fd, struct rl *rlbuf) {
	rlbuf->fd = fd;
	rlbuf->size = 0;		
	rlbuf->c = 0;
}

/**
 * Provide a buffering mechanism to limit the number of read system calls
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
	if (rlbuf->c + 1 >= rlbuf->size) {
		rlbuf->c = 0; // reset to beginning of buffer 
		// read a chunck of data and scan for newline
		rlbuf->size = read(rlbuf->fd, rlbuf->buf, n);
		if (rlbuf->size == -1)
			return -1;
	}

	// iterate through the buffer until the next newline
	for (; rlbuf->c < rlbuf->size; rlbuf->c++) {
		if (rlbuf->buf[rlbuf->c] == '\n')
			break;
		if (cnt < n)	
			buffer[cnt++] = rlbuf->buf[rlbuf->c];
	}
	buffer[cnt] = '\0';

	return cnt;
}

