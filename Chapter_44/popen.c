#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/resource.h>

// map file descriptors to process, assume 100 descriptors will do
static int p_map[100]; 

/**
 * Implementation of popen(3), see man page for details
 * does not support signals
 */
FILE* popen(const char* command, const char* type) {
	pid_t id;
	int status;
	int fildes[2];
	FILE* wp, *rp;

	if (!type || (type[0] != 'r' && type[0] != 'w'))
		return NULL;
	if (pipe(fildes) == -1)
		return NULL;	

	switch (id = fork()) {
		case -1:
			return NULL;
		case 0:
			if (type[0] == 'w') {
				// close write and get input from pipe
				if (fildes[0] != STDIN_FILENO) {
					if (close(fildes[1]) == -1)
						_exit(EXIT_FAILURE);
					if (dup2(fildes[0], STDIN_FILENO) == -1)
						_exit(EXIT_FAILURE);
					// close extra descriptor
					if (close(fildes[0]) == -1)
						_exit(EXIT_FAILURE);}
			} else {
				// close read end and redirect output to write end of pipe
				if (fildes[1] != STDOUT_FILENO) {
					if (close(fildes[0]) == -1)
						_exit(EXIT_FAILURE);
					if (dup2(fildes[1], STDOUT_FILENO) == -1) {
						_exit(EXIT_FAILURE);
					}
					// close extra descriptor
					if (close(fildes[1]) == -1)
						_exit(EXIT_FAILURE);
				}
			}
			
			if (system(command) == -1)
				_exit(EXIT_FAILURE);

			_exit(EXIT_SUCCESS);
			break;

		default:
			// parent closes uneeded pipe ends, record pid in map and
			// returns a stdio file pointer to the descriptor 
			if (type[0] == 'r') {
				close(fildes[1]); // close write end
				p_map[fildes[0]] = id;
				rp = fdopen(fildes[0], "r"); 
				return  rp;

			} else {
				close(fildes[0]); // close read end
				p_map[fildes[1]] = id;
				wp = fdopen(fildes[1], "w");
				return wp;
			}
			
			

	}	
}

/**
 *  Implmenetation of popen(3), see man page for details
 */
int pclose(FILE* stream) {
	int status;
	struct rusage ru;
	
	// wait on process with pid = id
	pid_t id = fileno(stream);
	if (wait4(id, &status, WEXITED, &ru) == -1) {
		return -1;
	}
	fclose(stream);

	return status;
}

/**
 * Driver function to test popen
 */
int main() {
	FILE* fp = popen("ls -l && sleep 3", "r");
	char buf[10000];	
	if (!fp) {
		perror("popen");
		return -1;
	}
	while (fgets(buf, 10000, fp) != NULL) {
		printf("%s", buf);
	}
	pclose(fp); // blocks
	printf("done\n");
}
