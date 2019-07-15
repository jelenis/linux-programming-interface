#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/stat.h>


// driver program for exercise
int main(int argc, char* argv[]) {
	int fd = open("test.txt", O_APPEND | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);
	int newfd = dup(fd);	
	int newfd2 = dup2(fd, 55);
	printf("fd: %d newfd: %d newfd2: %d\n", fd, newfd, newfd2);
	struct stat stat1, stat2, stat3;
	printf("%d %d %d\n", fstat(fd, &stat1), fstat(newfd, &stat2), fstat(newfd2, &stat3));
	printf("same ? %d %d %d\n", stat1.st_dev ,  stat2.st_dev ,  stat3.st_dev);
	printf("same other? %d %d %d\n", stat1.st_ino ,  stat2.st_ino ,  stat3.st_ino);
	return 0;
}

int dup(int oldfd) {

  	int newfd = fcntl(oldfd, F_DUPFD, 0);
	return newfd;
}

int dup2(int oldfd, int newfd) {
	if (newfd != oldfd) {
		close(newfd); 
		fcntl(oldfd, F_DUPFD, newfd);
	} else if (fcntl(oldfd, F_GETFL) == -1) {
		return -1;
	}	

	return newfd;
}
