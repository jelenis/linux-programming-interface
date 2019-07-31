#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<fcntl.h>
#include<errno.h>
#include<sys/stat.h>

/**
 * check if the effective user has permission to open the file
 * the systemcall equalvilent is euidaccess(3)
 */
int has_access(const char* pathname, int mode) {
	//TODO: use getpwnam and determine if the file has permissions
	// using stat and each permission bit
	// also check if and of the group ids have perimission

	//get effective id
	//uid_t euid = geteuid();	
	int flags = 0;
	if (mode & R_OK && mode & W_OK) flags |= O_RDWR; 
	else if(mode & R_OK) flags |= O_RDONLY;
	else if(mode & W_OK) flags |= O_WRONLY;
	int fd = open(pathname, 0, flags);
	if (fd == -1) {
		if (mode & F_OK && errno == ENOENT) return -1;	
		else if (errno == EACCES) return -1;
	}	
	close(fd);
	return 1;
}

/**
 * Driver for has_access function
 */
int main () {
	printf("%d\n", has_access("test.txt", F_OK));
	
	return 0;
}
