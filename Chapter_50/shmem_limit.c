#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <string.h>
#include <sys/resource.h>


/**
 * Demonstrates the effect of exceeding RLIMIT_MEMLOCK on locked memory mapping
 */
int main(int argc, char *argv[]) {
	long limit; 
	void *base;
	struct rlimit rlim;
	if (argc < 2) {
		printf("Missing new RLIMIT_MEMLOCK\n");
		return -1;
	}

	if (getrlimit(RLIMIT_MEMLOCK, &rlim) == -1) {
		perror("getrlimit");
		return -1;
	}
	printf("previous limit = %d\n", rlim.rlim_max);

	limit = strtod(argv[1], NULL);
	rlim.rlim_max = limit;
	rlim.rlim_cur = limit;
	if (setrlimit(RLIMIT_MEMLOCK, &rlim) == -1) {
		perror("setrlimit");	
		return -1;
	}
	
	// map and lock 1 byte greater than RLIMIT_MEMLOCK
	base = mmap(NULL, limit+1, PROT_READ, MAP_ANONYMOUS | MAP_PRIVATE | MAP_LOCKED, -1, 0);
	if (base == (void*)-1) {
		perror("mmap");
		return -1;
	}


	return 0;
}
