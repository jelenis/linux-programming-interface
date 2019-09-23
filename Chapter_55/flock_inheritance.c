#include <stdio.h>
#include <sys/file.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <wait.h>

/**
 * Demonstrates how the BSD locks are bound to file descriptors not the process.
 */
int main() {
	int fd;
	int perms = S_IRUSR | S_IWUSR;

	fd = open("test", O_RDWR | O_CREAT, perms);
	if (fd == -1) {
		perror("Parent open");
		_exit(EXIT_FAILURE);
	}
	// block on childs lock
	if (flock(fd, LOCK_EX) == -1) {
		perror("Parent flock");
		exit(EXIT_FAILURE);
	}

	switch (fork()) {
		case -1:
			exit(EXIT_FAILURE);
			close(fd); // this should remove the lock

			fd = open("test", O_RDWR | O_CREAT, perms);
			if (fd == -1) {
				perror("Child open");
				_exit(EXIT_FAILURE);
			}
			// will not block since the child fd refers to the same lock
			if (flock(fd, LOCK_EX) == -1) {
				perror("Child flock");
				exit(EXIT_FAILURE);
			}

		case 0:

			_exit(EXIT_SUCCESS);
		default:
			break;
	}


	wait(NULL);

}

