#include <stdio.h>
#include <sys/msg.h>
#include <sys/ipc.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/sysmacros.h>

/**
 * Implementation of System V's ftok, see ftok(3) for details
 */
key_t ftok (const char* pathname, int proj_id) {
	struct stat sb;
	key_t key = (proj_id << 24);

	if (stat(pathname, &sb) == -1) {
		perror("stat");
		exit(EXIT_FAILURE);
	}
	key |= (minor(sb.st_dev) << 16);
	key |= (sb.st_ino & 0xFFFF);
	return key;
}

int main(int argc, char* argv[]) {
	int key;			

	if (argc < 2) {
		printf("Please enter a pathname\n");
		exit(EXIT_FAILURE);	
	}


	key = ftok(argv[1], 2);
	printf("%x\n", key);

	

}
