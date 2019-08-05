#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<sys/types.h>
#include<sys/xattr.h>
#include<unistd.h>

/**
 * Implmentation of setfattr(1).
 *
 * It requires that the file system be mounted with the user_xattr option.
 * Does not support escaped strings or offer command line help.
 */
int main(int argc, char* argv[]) { 
	int use_links = 0;
	int remove = 0;
	char* path;	
	char* name;
	char* value;

	if (argc < 3) {
		printf("Not enough arguments specified\n");
		return -1;
	}
	
	int opt;
	while ( (opt = getopt(argc, argv, "n:v:x:h:")) != -1) {
		switch(opt) {
			case 'n':
				remove = 0;
				name = optarg;
				break;	
			case 'v':
				value = optarg;
				break;
			case 'x':
				remove = 1;
				name = optarg;
				break;
			case 'h':
				use_links = 1;
				break;
			case '?':
			       	break;
		}
	}
	path = argv[argc-1];
	
	if (!use_links) {
		if(!remove && setxattr(path, name, value, strlen(value) + 1, 0) == -1) {
			perror("Setting attribute");
			return -1;
		} else if (removexattr(path, name) != -1) {
			perror("Removing attribute");
			return -1;
		}
	} else { // follow symbolic links
		if(!remove && lsetxattr(path, name, value, strlen(value) + 1, 0) == -1) {
			perror("Setting linked attribute");
			return -1;
		} else if (lremovexattr(path, name) != -1) {
			perror("Removing linked attribute");
			return -1;
		}
	}

	return 0;
}

