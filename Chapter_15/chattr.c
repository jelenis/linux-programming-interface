#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/ioctl.h>
#include<string.h>
#include<fcntl.h>
#include<limits.h>
#include<linux/fs.h> // use these flags for ext4

/**
 * Returns the the attribute ORed flags as specified by the command line
 * options documented in chattr(1)
 */
int parse_attr(char* add) {
	int attr = 0;
	for (int i = 0; i < strlen(add); i ++) {
		switch(add[i]) {
			case 'a':
				attr |= FS_APPEND_FL;
				break;
			case 'c':
				attr |= FS_COMPR_FL;
				break;
			case 'D':
				attr |= FS_DIRSYNC_FL;
				break;
			case 'i':
				attr |= FS_IMMUTABLE_FL;
				break;
			case 'j':
				attr |= FS_JOURNAL_DATA_FL;
				break;
			case 'A':
				attr |= FS_NOATIME_FL;
				break;
			case 'd':
				attr |= FS_NODUMP_FL;
				break;
			case 't':
				attr |= FS_NOTAIL_FL;
				break;
			case 's':
				attr |= FS_SECRM_FL;
				break;
			case 'S':
				attr |= FS_SYNC_FL;
				break;
			case 'T':
				attr |= FS_TOPDIR_FL;
				break;
			case 'u':
				attr |= FS_UNRM_FL;
				break;
		}
	} 
	return attr;
}


/**
 * Implements the use of chattr, without -R, -V or -v options
 * 
 * If multiple options are past as arguments, the last one is used
 * 	eg: chattr +D +ac 
 * 	this will add ac, D will be ignored 
 * Using the = options will ignore all other +/- options
 */
int main(int argc, char* argv[]) {
	int opt;
	if (argc < 2) printf("Enter options followed by a path\n");
	
	int opt_count = 12;
	char filename[PATH_MAX];
	char add[opt_count + 1];
	char del[opt_count + 1];
	char equals[opt_count + 1];

	filename[0] = '\0';
	add[0] = '\0';
	del[0] = '\0';
	equals[0] = '\0';	

	for (char** p = argv + 1; *p != NULL; p++) {
		if (*p[0] == '+') {
			strcpy(add, *p+1);	
		} else if (*p[0] == '-') {
			strcpy(del, *p+1);
		} else if (*p[0] == '=' ) {
			strcpy(equals, *p+1);
		} else {
			strcpy(filename, *p);
		}
	}	
	
	int attr;
	int fd = open(filename, 0, O_RDONLY);
	if (fd == -1) {
		perror("open");
		return -1;
	}


	if (equals[0] != '\0') { // if equals is specified, override +/- behaivour
		printf("%s\n", equals);
		attr = parse_attr(equals); 
		attr |= FS_EXTENT_FL; // not to be removed by chattr
	} else {
		if (ioctl(fd, FS_IOC_GETFLAGS, &attr) == -1) { // fetch current flags
			perror("get ioctl");
			return -1;
		}
		attr |= parse_attr(add);
		attr &= ~parse_attr(del);
	}
	
	// update current flags
	if (ioctl(fd, FS_IOC_SETFLAGS, &attr) == -1) {
		perror("set ioctl");
		return -1;	
	}

}
