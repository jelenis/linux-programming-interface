#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <string.h>
#include <fcntl.h>

/**
 * Implementation of cp(1) that uses mmap and memcpy instead of read and write
 */
int main(int argc, char *argv[]) {
	int src_fd, dest_fd;
	char *src, *dest;
	struct stat sb;

	// parse sourced and desitination from stdin 
	if (argc < 2) {
		printf("Missing pathname\n");
		exit(EXIT_FAILURE);
	}	
	else if (argc < 3) {
		printf("Missing destination\n");
		exit(EXIT_FAILURE);
	}
	src = argv[1];
	dest = argv[2];

	// open the src_fd file
	src_fd = open(src, O_RDONLY);
	if (src_fd == -1) {
		perror("open");
		exit(EXIT_FAILURE);
	}

	// create the dest file rw-r--r
	dest_fd = open(dest, O_RDWR | O_CREAT, S_IWUSR | S_IRUSR
		       	| S_IRGRP  | S_IROTH);
	if (dest_fd == -1) {
		perror("open");
		exit(EXIT_FAILURE);
	}
	// get the size of source file
	if (fstat(src_fd, &sb) == -1) {
		perror("stat");
		exit(EXIT_FAILURE);
	}	
	
	// map the source file into memory as read only
	void *src_ptr = mmap(NULL, sb.st_size, PROT_READ, MAP_PRIVATE, src_fd, 0);
	if (src_ptr == (void*) -1) {
		perror("mmap read");
		exit(EXIT_FAILURE);
	}

	// make the destination file large enough 
	if (ftruncate(dest_fd, sb.st_size) == -1) {
		perror("ftruncate");
		exit(EXIT_FAILURE);
	}

	// map the dest file into memory
	void *dest_ptr = mmap(NULL, sb.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, dest_fd, 0);
	if (dest_ptr == (void*) -1) {
		perror("mmap write");
		exit(EXIT_FAILURE);
	}
	
	// copyt the file
	memcpy(dest_ptr, src_ptr, sb.st_size);

	// ignore error
	close(src_fd);
	close(dest_fd);

	
}
