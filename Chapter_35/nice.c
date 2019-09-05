#include <stdio.h>
#include <sys/resource.h>
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
	int opt;
	int inc = 0;

	while ((opt = getopt(argc, argv, "n:")) != -1) {
		printf("%c", opt);
		if (opt == 'n') {
			inc = atoi(optarg);	
		}
	}		
	
	// set nice value, execed process will inherit this
	errno = 0;
	if (nice(inc) == -1 && errno != 0) {
		perror("nice");
		return -1;
	}	
	
	/* exec the command in shell, path will be at optind, the arguments to
	 * command will be at optind+1 */
	if (execv(argv[optind], &argv[optind+1]) == -1) {
		perror("execv");
		return -1;
	}
	return 0;
}
