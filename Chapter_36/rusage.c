#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <wait.h>
#include <sys/resource.h>
#include <sys/times.h>


/**
 * Displays time rusage stats about program, analog to time(1)
 */
int main(int argc, char* argv[]) {
	struct rusage usage;
	float user, system;

	if (argc < 2) {
		printf("Specify a program to run\n");
		_exit(EXIT_FAILURE);
	}

	pid_t pid = fork();
	switch (pid) {
		case -1:
			perror("fork");
			exit(EXIT_FAILURE);
		case 0:
			// exec command in child
			if (execvp(argv[1], argv+1) == -1) {
				perror("execv");
				_exit(EXIT_FAILURE);			
			}
		default:
			wait(NULL);
					
			getrusage(RUSAGE_CHILDREN, &usage);
			
			user = usage.ru_utime.tv_sec + (float)usage.ru_utime.tv_usec/1000000;
			system = usage.ru_stime.tv_sec + (float)usage.ru_stime.tv_usec/1000000;
			printf("%lu\n", usage.ru_utime.tv_sec);

			printf("real\t%.3fs\n", user + system);	
			printf("user\t%.3fs\n", user);	
			printf("system\t%.3fs\n", system);	
	}
}
