#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <string.h>
#include <errno.h>

static char buf[PATH_MAX];

/**
 * Scans the directory in root for a character devices that has the
 * same device id as fd.
 * Returns the name of the file.
 */
char *matchfd (const char *root, int fd) {
	static struct stat sb;
	struct dirent* dent;
	char dirpath[PATH_MAX];
	DIR* dirp;
	dev_t dev;

	strcpy(dirpath, root);
	dirp = opendir(dirpath);
	if (fstat(fd, &sb) == -1)
		return NULL;
	dev = sb.st_rdev;	

	while (dent = readdir(dirp)) {
		char filepath[PATH_MAX];
		if (dent->d_type != DT_CHR)
			continue;	

		/* append the filename to the root and check if the 
		   device id of fd is the same as the files */
		sprintf(filepath, "%s/%s", dirpath, dent->d_name);
		if (stat(filepath, &sb) == -1)
			continue;

		if (dev == sb.st_rdev) {
			strcpy(buf, filepath);
			return buf;
		}
	}

	errno = ENOENT;
	return NULL;
}
/**
 * Simple implementation of ttyname(3), see man page for
 * more details.
 * Returns the name of terminal or NULL on error.
 */
char *ttyname(int fd) {
	char *res;
	if ((res = matchfd("/dev/pts", fd)) != NULL) 
		return res;
	return matchfd("/dev", fd);
}


int main() {
	char *b = ttyname(0);
	if (b == NULL) {
		perror("ttyname");
		return -1;
	}
	printf("%s\n", b);	
}
