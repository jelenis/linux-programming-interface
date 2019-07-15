#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

int main(int argc, char* argv[]) {
	
	int fd = open(argv[1], O_RDWR | O_APPEND | O_CREAT,
		       	S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH );
	
	// seek to the beggining of the file
	lseek(fd, 0, SEEK_SET);
	
	// writing will still be appended even though the seek has moved the pointer
	// to the end of the file
	int numWrite = write(fd, argv[2], strlen(argv[2]) );
	if (numWrite == -1) {
		printf("%s\n", strerror(errno));
	}

	return 0;
}
