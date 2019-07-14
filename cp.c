
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h> 
#include <stdlib.h>
#include <errno.h> 



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

	dest_fd = open(argv[2], O_CREAT | O_WRONLY);
	if (dest_fd == -1) {
		printf("%s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}

	ssize_t numWritten, numRead;
	int len = 1000; // 1 MB
	char * buf = malloc(len+1);

	while((numRead = read(src_fd, buf, len)) > 0 ){
		buf[numRead] = '\0';
		printf("%s\n",buf);
		numWritten = write(dest_fd, buf, numRead); // write the string excluding '\0'
	}
	if (numRead == -1) {
		printf("%s\n", strerror(errno));
	}	

	if (buf != NULL) free(buf);
	close(src_fd);
	close(dest_fd);	
	return 0;
}
