#define _GNU_SOURCE

#include <stdio.h>
#include <sys/resource.h>
#include <sys/times.h>
#include <sched.h>
#include <unistd.h>
#include <stdlib.h>
#include <limits.h>
#include <sys/wait.h>
#include <errno.h>

static void consume_time() {
	clock_t start = times(NULL);
	clock_t end = start;
	int quarters = 1, seconds = 1;
	long tick_per_second = sysconf(_SC_CLK_TCK);
	float duration = 0;
	pid_t pid = getpid();

	while(duration < 3) {
		end = times(NULL);	
		duration = (float)(end - start)/tick_per_second;
		
		// quarter of a second
		if (duration >= 0.25*quarters) {
			printf("PID: %ld time: %.2f s\n", pid, duration);		
			quarters ++;
		}

		// yield every second
		if (duration >= 1*seconds) {
			printf("yield\n");
			sched_yield();
			start = times(NULL);
			seconds ++;
		}
	}
}
/**
 * Runs a command with adjust niceness value, see nice(1)
 * for details.
 */
int main(int argc, char* argv[]) {
	int policy;
	int priority = 0;

	// ensure root access
	if (seteuid(0) == -1) {
		perror("seteuid");	
		exit(EXIT_FAILURE);
	}

	// set scheduling with root access
	struct sched_param param;
	param.sched_priority = 1;
	if (sched_setscheduler(0, SCHED_FIFO, &param) == -1) {
		perror("sched_setscheduler");
		exit(EXIT_FAILURE);
	}
	
	int pid = fork();
	cpu_set_t set;
	CPU_ZERO(&set);
	CPU_SET(0, &set);	

	sched_setaffinity(0, sizeof(cpu_set_t), &set);
	switch(pid) {
		case -1:
			perror("fork");
			exit(EXIT_FAILURE);
			break;
		case 0: 
			consume_time();
			_exit(EXIT_SUCCESS);
			break;
		default:
			consume_time();
			wait(NULL);
			exit(EXIT_SUCCESS);
			break;
	}

	return 0;
}
