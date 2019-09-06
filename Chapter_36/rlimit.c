#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <wait.h>
#include <sys/resource.h>
#include <sys/times.h>


/**
 * Creates the max number of children
 */
int main(int argc, char* argv[]) {

	printf("Max number of children: %ld\n", sysconf(_SC_CHILD_MAX));
 	for (int i = 0; i < sysconf(_SC_CHILD_MAX); i++) {
		printf("child: %d\n", i);
		pid_t pid = fork();
		switch (pid) {
			case -1:
				perror("fork");
				exit(EXIT_FAILURE);
			case 0:
				_exit(EXIT_SUCCESS);
		}
	}
}
