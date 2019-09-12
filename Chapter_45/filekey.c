#include <stdio.h>
#include <sys/msg.h>
#include <sys/ipc.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/sysmacros.h>

/**
 * Demonstrates that System V file key is made from the 
 * minor device id and inode number
 */
int main(int argc, char* argv[]) {
	int key;			
	struct stat sb;

	if (argc < 2) {
		printf("Please enter a pathname\n");
		exit(EXIT_FAILURE);	
	}

	if (stat(argv[1], &sb) == -1) {
		perror("stat");
		exit(EXIT_FAILURE);
	}

	key = ftok(argv[1], 1);
	if (key == -1) {
		perror("ftok");
		exit(EXIT_FAILURE);
	}

	printf("inode: %x minor device: %x  key: %x\n", sb.st_ino, minor(sb.st_dev), key);
	

}
