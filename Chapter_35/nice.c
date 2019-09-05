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
	int num = 0;
	//char* buf = malloc(_POSIX_ARG_MAX);
	char buf[_POSIX_ARG_MAX];
	buf[0] = '\0'; // empty string

	while ((opt = getopt(argc, argv, "n:")) != -1) {
		if (opt == 'n') {
			num = atoi(optarg);	
		}
	}		
	
	// concatinate the rest of argv arguments into a single command in buf	
	for (int i = optind; i < argc; i++) {
		sprintf(buf, "%s %s", buf, argv[i]);	
	}

	// set nice value, child process will inherit this
	errno = 0;
	if (nice(num) == -1 && errno != 0) {
		perror("nice");
		return -1;
	}	
	
	// execute the command in system shell 
	if (system(buf) == -1) {
		return -1;
	}
	//free(buf);
	return 0;
}
