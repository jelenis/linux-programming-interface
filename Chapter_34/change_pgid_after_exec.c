#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
/**
 * Demonstrates that a parent process cannot change PGID of a child after
 * an exec. Uses sleep, so race condition is present, however this should
 * suffice for example purposes
 */
int main (int argc, char* argv[]) {

	pid_t p = fork();	
	if (p == -1) {
		perror("fork");
		return -1;
	}
	else if (p == 0) { // child
		sleep(2); // allow parent to change PGID, race condition
		// peform exec (run ps command to show PGID)
		execl("/bin/sh", "sh", "-c", "ps -jf", (char *) NULL);
		sleep(10); // wait for parent
	} else {
		if (argc == 1) {
			// place child in a new process group before
			if (setpgid(p, p) == -1) {
				perror("setpgid");
				return -1;
			}	
		} else {
			// attempt to chage PGID after, will be error
			sleep(4);
			if (setpgid(p, p) == -1) {
				// should print Permission Denied
				perror("setpgid");
				return -1;
			}
		}
		pause();
	}
}
