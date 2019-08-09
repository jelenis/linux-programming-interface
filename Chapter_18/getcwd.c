#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>

/**
 * Basic implementation of getcwd(2) using readdir and stat to locate filenames
 * by their inodes
 */
char* getcwd(char* buf, size_t size) {
	char* files[256];
	struct dirent* entryp;
	DIR* dirp; 
	struct stat cur_sb;
	struct stat entry_sb; 
	ino_t prev;
	int len = 0;

	// save the current working directory file descriptor
	int fd = open(".", O_RDONLY, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);	
	if (fd == -1)
		return NULL;

	if (stat(".", &cur_sb) == -1)
		return NULL;

	if (chdir("..") == -1)
		return NULL;	

	dirp = opendir(".");
	if (dirp == NULL) 
		return NULL;

	/* Walk through current parent directory to locate which filename has the same
	 * inode as the previous child directory. Repeat this process until the parent
	 * directory is the same as the current direcotry. */
	while (cur_sb.st_ino != prev) {
		
		// search all child inodes
		while ((entryp = readdir(dirp)) != NULL) {

			if (entryp->d_ino == cur_sb.st_ino != 0) {
				/* also compare the dev id to makse sure this is the
				 * file system */
				if (stat(entryp->d_name, &entry_sb) == -1)
					return NULL;
				else if (entry_sb.st_dev == cur_sb.st_dev) {
					files[len] = strdup(entryp->d_name);				
					len ++;
					prev = entryp->d_ino;
					break;
				}

			}
		}
		if (closedir(dirp)== -1)
			return NULL; 

		// record the current directory's stats
		if (stat("./", &cur_sb) == -1)
			return NULL;
		
		// move into parent direcotry	
		if (chdir("../") == -1)
			return NULL;
		dirp = opendir("./");
		if (dirp == NULL) {
			return NULL;
		}
	}	
	if (closedir(dirp) == -1)
		return NULL;
	
	/* concactinate each string in the files array and add
	 * forward slashes between */
	char* cur = buf;	
	for (int i = len - 1; i >= 0; i--) {
		printf("%s\n", files[i]);
		for (char* p = files[i]; *p != '\0'; p++)
			*cur++ = *p;
		*cur++ = '/';
		free(files[i]);
	}
	*(--cur) = '\0';

	// restore current working directory
	fchdir(fd);

	return buf;
}

/**
 * Driver program for getcwd(2)
 */
int main(int argc, char* argv[]) {
	char buf[PATH_MAX];

	if (getcwd(buf, PATH_MAX) == NULL) {
		perror("getcwd");
		return -1;
	}
	printf("%s\n", buf);
	return 0;
}
