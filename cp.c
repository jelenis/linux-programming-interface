
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h> 
#include <stdlib.h>

int main(int argc, char* argv[]) {
		
	int src_fd;
	int dest_fd;

	src_fd = open(argv[1], O_RDONLY);
	dest_fd = open(argv[2], O_CREAT | O_WRONLY);
//	numWritten = write(fd, buf, numRead); // write the string excluding '\0'


	close(src_fd);
	clos(dest_fd);	
	return 0;
}
