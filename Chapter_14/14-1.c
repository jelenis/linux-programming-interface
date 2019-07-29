#include<stdio.h>
#include<stdlib.h>
#include<limits.h>
#include<string.h>
#include<errno.h>
#include<sys/time.h>
#include<time.h>
#include<unistd.h>

/*
 * Returns the time needed to create and destory file_count files 
 * in micro-seconds
 */
int testfs(char* template, int file_count, int inorder) {
	
	char** filenames = malloc(file_count*sizeof(char*));	
	struct timeval start;
	gettimeofday(&start, NULL);
	
	for(int i = 0; i < file_count; i++) {
		int fd;
		filenames[i] = malloc((strlen(template) + 1));
		strcpy(filenames[i], template);
		if ( (fd = mkstemp(filenames[i])) == -1) {
			perror("Error creating files");
			exit(EXIT_FAILURE);
		} else {
			write(fd, "A", 1);
			close(fd);
		}	
	}	
	
	// delete in order of creation if an extra argument is specified
	// else remove backwards (ie a different order)
	if (inorder) {
		for(int i = 0; i < file_count; i++) {
			if (remove(filenames[i]) == -1) {
				perror("Error removing files");
				exit(EXIT_FAILURE);
			}
			free(filenames[i]);
		} 
	} else {
		for (int i = file_count - 1; i >= 0; i--) {
			if (remove(filenames[i]) == -1) {
				perror("Error removing files");
				exit(EXIT_FAILURE);
			}
			free(filenames[i]);
		}	
	}

	struct timeval end;
	gettimeofday(&end, NULL);	
	
	int res = 0;
	res += 1000000*(end.tv_sec - start.tv_sec);
	res += end.tv_usec - start.tv_usec;
	free(filenames);	
	return res;

}
/**
 * Test the speed of creating and removing files
 * when removing them in both the order they were
 * created and not 
 *
 * For file systems like ext2 with a block size of 4096
 * we should see that there is faster to remove files in order
 * since they would be removed from the front of the linked list 
 **/
int main(int argc, char* argv[]) {
	int file_count;
	
	char* f = "xXXXXXX";
	char template[PATH_MAX];

	if (argc < 3) {
		printf("Enter path followed by number of files\n");
		return -1;
	}
	// use 
	if (argv[1][0] != '.') strcpy(template, argv[1]);	
	else strcpy(template, argv[1] + 2);

	strcat(template, f);
	file_count = strtol(argv[2], NULL, 10);
	long sum = 0;
	int count = 0;
	for (int i = 0; i < 10; i ++) {	
		int res = testfs(template, file_count, argc == 4);
		sum += res;
		printf("%d\n", res);
		count ++;
	}

	printf("average elapsed time %ld \xC2\xB5s\n", sum/count);
}
