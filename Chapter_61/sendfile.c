#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

/**
 * Implementation of sendfile(2) in the userspace
 */
ssize_t sendfile(int out_fd, int in_fd, off_t *offset, size_t count) {
	long ps = sysconf(_SC_PAGESIZE);
	unsigned char buf[ps];
	ssize_t num_read;
	ssize_t tot = 0;
	
	if (lseek(in_fd, *offset, SEEK_SET) == -1)
		return -1;
	while ( (num_read = read(in_fd, buf, count)) > 0) {
		write(out_fd, buf, num_read);
		tot += num_read;
	}
	*offset += tot + 1;

	return tot;
}
