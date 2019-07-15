#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h> 
#include <stdlib.h>
#include <errno.h> 
#include <stdlib.h>

/** 
 * A simple implmentation of the cp program.
 * Copies from src to dest streams 1MB at a time
 * */
int main(int argc, char* argv[]) {		
	int src_fd;
	int dest_fd;
	
	if (argc < 3) {
	  printf("Please enter as src followed by a dest\n");
	  return 0;
	}

	src_fd = open(argv[1], O_RDONLY);
	if (src_fd == -1) {
		printf("%s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}
	
	// open with rw-rw-rw-
	dest_fd = open(argv[2], O_CREAT | O_WRONLY,
		       	S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);
	if (dest_fd == -1) {
		printf("%s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}

	// Use up to 1 MB blocks for copying
	int len = 1000; 
	ssize_t numWritten, numRead;
	char * buf = malloc(len);

	// read while the file is not empty
	while((numRead = read(src_fd, buf, len)) > 0 ){
		numWritten = write(dest_fd, buf, numRead); 
	}
	if (numRead == -1) {
		printf("%s\n", strerror(errno));
	}

		
	if (buf != NULL) free(buf);

	close(src_fd);
	close(dest_fd);	
	return 0;
}
