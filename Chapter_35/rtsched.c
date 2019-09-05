#include <stdio.h>
#include <sys/resource.h>
#include <sys/time.h>
#include <sched.h>
#include <unistd.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>
#include <errno.h>

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
		return -1;
	}

	if (argc < 3) {
		printf("Usage: policy priority command ...arg\n");
		return -1;
	}
	
	/* get priority and policy from the cammand line */
	switch (argv[1][0]) {
		case 'f':
			policy = SCHED_FIFO;
			break;
		case 'r':
			policy = SCHED_RR;
			break;
	}
	
	priority = strtol(argv[2], NULL, 10);

	// set scheduling with root access
	struct sched_param param;
	param.sched_priority = priority;
	if (sched_setscheduler(0, policy, &param) == -1) {
		perror("sched_setscheduler");
		return -1;
	}
	

	// restore real uid and permanently drop root access
	if (setuid(getuid()) == -1) {
		perror("setuid");
		return -1;
	}	

	// exec command
	if (execv(argv[3], argv+4) == -1) {
		perror("execv");
		return -1;
	}

	return 0;
}
