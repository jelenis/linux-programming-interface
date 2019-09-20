#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/mman.h>

/**
 * Creates a nonlinear mapping using the MAP_FIXED flag (this is less efficient than using the
 * linux specifc remap_file_pages) uses the file specified in the command line
 *
 * 'a', 'b' and 'c' are filled in pages 1, 2, 3 in the input file, this program then remaps
 * pages in process's virutal memory so that 1 maps to page 3 and 3 maps to page 1 of the file.
 * It then prints the processes sequential view of each page. The result is that the
 * characters are printed in the swapped order of c, b, a.
 */
int main(int argc, char *argv[]) {
	int fd;
	char *base, *first, *second, *third;
	long ps = sysconf(_SC_PAGESIZE);	

	if (argc < 2) {
		printf("filename is missing\n");
		exit(EXIT_FAILURE);
	}

	fd = open(argv[1], O_RDWR);
	if (fd == -1) {
		perror("open");
		exit(EXIT_FAILURE);
	}

	// allocate 3 pages worth of space in the file
	if (ftruncate(fd, ps*3) == -1) {
		perror("ftruncate");
		exit(EXIT_FAILURE);
	}	

			

	// create normal anonmous mapping for all 3 bytes
	base = mmap(NULL, 3*ps, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	if (base == (void*)-1) {
		perror("mmap");
		exit(EXIT_FAILURE);
	}

	printf("Writing the following values to each page:\n");
	printf("first = '%c' second = '%c' third =  '%c'\n", *base, *(base + ps), *(base + ps*2));
	memset(base, 'a', ps);
	memset(base + ps, 'b', ps);
	memset(base + ps*2, 'c', ps);

	/* remap the first page of the file to the third virtual page 
	   0 ----> base + 2*ps  */
	first = mmap(base + 2*ps, 1, PROT_READ | PROT_WRITE, MAP_FIXED
		       	| MAP_SHARED, fd, 0);
	if (first == (void*)-1) {
		perror("first remap");
		exit(EXIT_FAILURE);
	}

	/* The second page of the file is mapped to the second page of virtual memory 
	   this was already done with the original mapping 
	   ps ----> base + ps */
	second = base + ps;

	/* remap the third file page to the first virtual page
	   ps*2 ----> base */
	third  = mmap(base, 1, PROT_READ | PROT_WRITE, MAP_FIXED
		       | MAP_SHARED , fd, ps*2);
	if (third == (void*)-1) {
		perror("third remap");
		exit(EXIT_FAILURE);
	}
	
	printf("\n");
	printf("Remapped pages are as follows:\n");
        printf("page1 = '%c', page2 = '%c', page3 = '%c'\n", base[0], base[ps], base[2*ps]);


}
