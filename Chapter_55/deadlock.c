#include <stdio.h>
#include <sys/file.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <wait.h>

/**
 * Demonstrates that BSD flock locks do not detect dead lock over multiple files.
 */
int main() {
	int fd;
	int perms = S_IRUSR | S_IWUSR;

	switch (fork()) {
		case -1:
			exit(EXIT_FAILURE);

		case 0:
			sleep(1); // should be enough to let parent finish
			fd = open("test2", O_RDWR | O_CREAT, perms);
			if (fd == -1) {
				perror("Child open");
				_exit(EXIT_FAILURE);
			}
			if (flock(fd, LOCK_EX) == -1) {
				perror("Child flock");
				_exit(EXIT_FAILURE);
			}		
			
			fd = open("test1", O_RDWR | O_CREAT, perms);
			if (fd == -1) {
				perror("Child open");
				_exit(EXIT_FAILURE);
			}

			// block on parents lock
			if (flock(fd, LOCK_EX) == -1) {
				perror("Child flock");
				_exit(EXIT_FAILURE);
			}			

			_exit(EXIT_SUCCESS);
		default:
			break;
	}
	
	fd = open("test1", O_RDWR | O_CREAT, perms);	
	if (fd == -1) {
		perror("open");
		_exit(EXIT_FAILURE);
	}
	if (flock(fd, LOCK_EX) == -1) {
		perror("Parent flock");
		exit(EXIT_FAILURE);
	}
	sleep(2); // child will lock test2

	fd = open("test2", O_RDWR, perms);	
	if (fd == -1) {
		perror("open");
		_exit(EXIT_FAILURE);
	}

	// block on childs lock
	if (flock(fd, LOCK_EX) == -1) {
		perror("Parent flock");
		exit(EXIT_FAILURE);
	}

	wait(NULL);

}

