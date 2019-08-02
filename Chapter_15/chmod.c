#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/stat.h>

/**
 * Emulates the terminal command: chmod a+rX [file|dir] [file|dir] ...
 */
int main(int argc, char* argv[]) {

	if (argc < 2) printf("Please enter paths to each file\n");
	
	for (char** p = argv+1; *p != NULL; p++) {
		mode_t mode;
		struct stat statbuf;	

		// get current permissions from stat
		if (stat(*p, &statbuf) == -1)	{
			return -1;
		}
		mode = statbuf.st_mode;

		// enable read permissions for all categories
		mode |= S_IRUSR | S_IRGRP | S_IROTH;

		// check if regular file using macro and if x bit is set
		// or if file is a directory
		if ((S_ISREG(mode) && (mode & S_IXUSR) == S_IXUSR) || S_ISDIR(mode)) {
			// add all execute permissions for dirs
			mode |= S_IXUSR | S_IXGRP | S_IXOTH;
		} else if (!(S_ISREG(mode) || S_ISDIR(mode))) {
			printf("paths must lead to a file or a directory\n");
			return -1;
		}	

		if (chmod(*p, mode) == -1) {
			perror("chmod");
			return -1;
		}
	}
}
