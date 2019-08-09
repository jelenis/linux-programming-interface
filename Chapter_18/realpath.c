#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <limits.h>
#include <string.h>
#include <fcntl.h>
#include <stdlib.h>

/**
 * Basic implementation of realpath(3)
 */
char* realpath(const char* path, char* resolved_path) {	
	char* tok;
	char* p1 = resolved_path; // use pointers to keep track of index
	char path_copy[PATH_MAX]; // copy path to tokenize non-desctructivley
	strcpy(path_copy, path);	

	if (resolved_path == NULL) {
		resolved_path = malloc(PATH_MAX);
	}	
	// handle relative path
	if (path[0] != '/') { 
		if (getcwd(resolved_path, PATH_MAX) == NULL)
			return NULL;
		p1 += strlen(resolved_path);
	}

	// append traling slash 
	*p1 = '/';
	*(++p1) = '\0';
	
	/* Tokenize the path_copy at each forward slash.
	 The entire path will be replaced if its a symbolic link.
	 If the token is .. the last filename will be removed from the 
	 the string  */	
	tok = strtok(path_copy, "/");	
	while (tok != NULL) {
		char* p2  = tok;
		char deref[PATH_MAX];

		// handle ../ when the parent is not root itself
		if (p1 - 1 != resolved_path && strcmp(tok, "..") == 0) {
			// trim off the last forward slash
			*(--p1) = '\0';

			/* get the position of previous filename from the end of 
			 * the string */
			p1 = strrchr(resolved_path ,'/');
			*(++p1) = '\0';
		} else {
			// concatinate and keep pointer reference to end
			// of the string
			for (char* p2 = tok; *p2 != '\0'; p2++) {
				*p1 = *p2;  	
				p1++;
			}	
			*p1 = '\0'; 

			printf("testing: %s\n",resolved_path);		
			ssize_t nbytes = readlink(resolved_path, deref, PATH_MAX);

			/* The resolved path is a symlink, replace the entire path
			 and update the character pointer posistion */
			if (nbytes != -1) {
				strcpy(resolved_path, deref);	
				p1 = resolved_path + nbytes;
			}			

			// append trailing slash to end of string
			*p1 = '/';
			*(++p1) = '\0';
		}
		tok = strtok(NULL, "/");
	}		

	return resolved_path;
}

int main(int argc, char* argv[]) {
	if (argc < 2) {
		printf("testing realpath requires a path input argument\n");
		return -1;
	}

	char resolved_path[PATH_MAX];	
	realpath(argv[1], resolved_path);
	return 0;
}
