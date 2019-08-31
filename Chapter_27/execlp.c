#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>
#include <string.h>

extern char** environ;

/**
 * implmentation of execlp, see execlp(3) for details
 */
int execlp(const char* file, const char* arg, ...) {
	char** argv = malloc(sizeof(char*));	
	char* path;
	char* tok;	
	path = getenv("PATH");

	// convert arglist to argv
	size_t i = 0;
	if (arg != NULL) {
		argv[i] = malloc(strlen(arg) + 1);
		strcpy(argv[i], arg);
		i++;
	}

	va_list ap;
	va_start(ap, arg);
	while (*arg != '\0') {
		
		char* s = va_arg(ap, char*);	
		if (s != NULL) {
			argv[i] = malloc(strlen(s) + 1);	
			strcpy(argv[i], s);
			i++;
		}
		arg++;
	}
	va_end(ap);
	// terminate argv with null
	argv[i] = (char*)NULL;	

	if (file[0] != '/') {
		// try to exec on each path
		for (char* str = path; ; str = NULL) {
			char full_path[PATH_MAX];
			tok = strtok(str, ":");
			if (tok == NULL)
				break;
			strcpy(full_path, tok);
			strcat(full_path, "/");
			strcat(full_path, file);
			// try this path and ignore errors
			execve(full_path, argv, environ);			
		}
	} else {
		// use abosolute path
		execve(file, argv, environ);			
	}

	perror("exec");
	for (size_t j = 0; j < i; j++) {
		free(argv[j]);
	}
	free(argv);
}

/**
 * Driver program to test execlp
 */
int main() {
	// test execlp, use the /bin/sh binary 
	execlp("sh", "sh",  "-c", "echo test",  (char*) NULL);	
}
