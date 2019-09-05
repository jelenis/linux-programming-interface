#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <wait.h>
#include <sys/resource.h>
#include <sys/times.h>

void consume_cpu(int seconds) {
	clock_t start = times(NULL);	
	long tick_second = sysconf(_SC_CLK_TCK);
	
	// consume seconds of CPU time
	while((times(NULL) - start)/tick_second < seconds);
}

/**
 * Verifies that resource stats are only returned on waited for children
 */
int main(int argc, char* argv[]) {
	struct rusage usage;
	pid_t pid = fork();

	switch (pid) {
		case -1:
			perror("fork");
			exit(EXIT_FAILURE);
		case 0:
			consume_cpu(5);
			break;
		default:
			// wait is specified, else let child become zombie
			sleep(1); // let child run
			getrusage(RUSAGE_CHILDREN, &usage);

			/* there will be no usage information since the child
			is not waited on */
			printf("USER: %ld KERNEL: %ld\n", usage.ru_utime.tv_sec,
				usage.ru_stime.tv_sec);

			wait(NULL);
			getrusage(RUSAGE_CHILDREN, &usage);

			// this will print usage since the child was waited on
			printf("USER: %lld KERNEL: %lld\n", usage.ru_utime.tv_sec,
				usage.ru_stime.tv_sec);
	}
	
}
