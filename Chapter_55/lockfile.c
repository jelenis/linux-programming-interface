/**
 * Simple implementation of lockfile with minimal features.
 */

#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <signal.h>
#include <sys/stat.h>

static void timeout(int signum) {}

/**
 * Places a lock on a file
 */
int main(int argc, char *argv[]) {
	int fd;
	struct flock fl;
	struct sigaction sa;

	// establish handler for alarm timeout
	sa.sa_handler = timeout;
	sa.sa_flags = 0;
	sigaction(SIGALRM, &sa, NULL);

	fl.l_type = F_WRLCK; 
	fl.l_whence = SEEK_SET;
	fl.l_start = 0;
	fl.l_len = 0; // end of file
	
	if (argc < 3) {
		fprintf(stderr, "lock file: Missing argument\n");
		return -1;
	}

	fd = open(argv[1], O_CREAT | O_RDONLY, S_IWUSR | S_IRUSR);
	if (fd == -1) {
		perror("open");
		return -1;
	}

	// set a timeout alarm
	alarm(argc > 2 ? strtod(argv[2], NULL) : 0);

	// attempt to aquire file lock
	if (fcntl(fd, F_SETLKW, &fl) == -1) {
		if (errno != EINTR) 
			return -1;
	}

	// clear alarm
	alarm(0);
	return 1;
}

