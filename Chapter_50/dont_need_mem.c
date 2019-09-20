#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <string.h>


/**
 * Demonstrates thev effect of MADV_DONTNEED
 * on madvise(2)
 */
int main(int argc, char *argv[]) {
	int *base;
	long ps;
	char vec;

	ps = sysconf(_SC_PAGESIZE);

	base = mmap(NULL, ps, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
	if (base == (void*)-1) {
		perror("mmap");
		return -1;
	}

	base[0] = 1;
	
	// determine if the page is in RAM
	if (mincore(base, ps, &vec) == -1) {
		perror("mincore");
		return -1;
	}
	printf("Before MADV_DONTNEED the page %s in memory\n", (vec & 1) ? "is" : "is not");
	if (madvise(base, ps, MADV_DONTNEED) == -1) {
		perror("madvise");
		return -1;
	}

	// determine if the page is in RAM after the kernel has been told to remove it
	if (mincore(base, ps, &vec) == -1) {
		perror("mincore");
		return -1;
	}
	printf("After MADV_DONTNEED the page %s in memory\n", (vec & 1) ? "is" : "is not");



	return 0;
}
