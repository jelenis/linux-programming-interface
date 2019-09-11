#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wait.h>
#include <time.h>

/**
 * Compares the speed of pipes
 */
int main(int argc, char* argv[]) {
	int fildes[2];	
	char* block;
	int num;
	ssize_t size;

	if (argc < 3) {
		printf("Enter number of blocks followed by size of each block\n");
		exit(EXIT_FAILURE);
	}	

	num = strtol(argv[1], NULL, 10);
	size = strtol(argv[2], NULL, 10);
	block = malloc(size);
	

	int status = pipe(fildes);
	if (status == -1) {
		perror("pipe");
		exit(EXIT_FAILURE);
	}

	char* buf = malloc(sizeof(block));

		
	struct timespec start, end;
	clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &start);
		
	switch (fork()) {
		case -1:
			break;

		case 0: // child reads
			close(fildes[0]);
			while (read(fildes[1], buf, sizeof(buf)) > 0);
			close(fildes[1]);
			_exit(EXIT_SUCCESS);
			
		default: // parent writes
			close(fildes[1]);
			for (int i = 0; i < num; i++)
				write(fildes[0], block, sizeof(block));
			close(fildes[0]); // child will see EOF
			wait(NULL);
			clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &end);
			double res = (end.tv_sec - start.tv_sec) * 1e3 
				+ (end.tv_nsec - start.tv_nsec) / 1e6; 	

			printf("time: %lf ms\t bytes: %ld Kb/s: %lf\n", res,
				       	size*num, size*num/res/1e6);
			break;
	}

}
