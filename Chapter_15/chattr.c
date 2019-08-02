#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/ioctl.h>
#include<string.h>
#include<fcntl.h>
#include<limits.h>
#include<linux/fs.h> // use these flags for ext4
/**
 * 
 */
int main(int argc, char* argv[]) {
	int opt;
	if (argc < 2) printf("Enter options followed by a path\n");
	
	int opt_count = 12;
	char filename[PATH_MAX];
	char add[opt_count];
	char del[opt_count];

	filename[0] = '\0';
	add[0] = '\0';
	del[0] = '\0';
	
	for (char** p = argv + 1; *p != NULL; p++) {
		if (*p[0] == '+') {
			strcpy(add, *p+1);	
		} else if (*p[0] == '-') {
			strcpy(del, *p+1);
		} else if (*p[0] == '=' ) {
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

	if(ioctl(fd, FS_IOC_GETFLAGS, &attr) == -1) { // fetch current flags
		perror("get ioctl");
		return -1;
	}

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

	for (int i = 0; i < strlen(del); i ++) {
		switch(del[i]) {
			case 'a':
				attr &= ~FS_APPEND_FL;
				break;
			case 'c':
				attr &= ~FS_COMPR_FL;
				break;
			case 'D':
				attr &= ~FS_DIRSYNC_FL;
				break;
			case 'i':
				attr &= ~FS_IMMUTABLE_FL;
				break;
			case 'j':
				attr &= ~FS_JOURNAL_DATA_FL;
				break;
			case 'A':
				attr &= ~FS_NOATIME_FL;
				break;
			case 'd':
				attr &= ~FS_NODUMP_FL;
				break;
			case 't':
				attr &= ~FS_NOTAIL_FL;
				break;
			case 's':
				attr &= ~FS_SECRM_FL;
				break;
			case 'S':
				attr &= ~FS_SYNC_FL;
				break;
			case 'T':
				attr &= ~FS_TOPDIR_FL;
				break;
			case 'u':
				attr &= ~FS_UNRM_FL;
				break;
		}
	} 
	if (ioctl(fd, FS_IOC_SETFLAGS, &attr) == -1) {// update current flags
		perror("set ioctl");
		return -1;	
	}

}
