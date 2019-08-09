#define _XOPEN_SOURCE 500
#include <stdio.h>
#include <ftw.h>
#include <stdlib.h>

int f = 0;
int d = 0;
int s = 0;

int walk(const char* fpath, const struct stat* sb, int type, struct FTW* ftwbuf) {

	switch(type){
		case FTW_F:
			f++;
			break;
		case FTW_D:
		case FTW_DNR:
			d++;
			break;
		case FTW_SL:
		case FTW_SLN:
			s++;
			break;
	}
	return 0;
}

/**
 * Simple program to walk to each directory in argv[1] and record the count 
 * of files, directories and symlinks
 */
int main(int argc, char* argv[]) {

	if (argc < 2) {
		printf("need a path to a directory\n");
		return -1;
	}
	
	if (nftw(argv[1], walk, 32 , FTW_PHYS) == -1) {
		perror("ntfw");
		return -1;
	}
	printf("files: %d directories: %d symlinks: %d\n", f, d, s);
	return 0;
}
