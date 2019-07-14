
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h> 
#include <stdlib.h>

int main(int argc, char* argv[]) {
		
	int fd, opt, append = 0;
	char* options = "a";	
	
	// simple check for arguments
	if (argc < 2 || argv[1][0] == '-') {
		printf("not enough arguments\n");
		return -1;
	}

	int flags =  O_RDWR | O_CREAT;
	while( (opt =  getopt(argc, argv, options)) != -1) {	
		if (opt == 'a') {
		 	flags |= O_APPEND;
		}		
	}
	
	
	fd = open(argv[argc-1], flags,  S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);

	if (fd == -1) {
		printf("could not open file\n");
		exit(-1);
	}



	// read from stdin
	ssize_t numWritten, numRead = 0;
	char buf[256]; // store 1 extra byte for '\0'
	size_t len = 0;
	char c;
	
	// read at most 255 bytes
	while ( (numRead = read(STDIN_FILENO,  buf, 255)) != -1) {
		buf[numRead] = '\0'; // end string at the end of the number read, needed for printing
		printf("%s", buf);
		numWritten = write(fd, buf, numRead); // write the string excluding '\0'

		if (numWritten == -1) {
			printf("could not write to file\n");			
		}
		printf("%d\n", numWritten);
	
	}

	close(fd);	
	return 0;
}
