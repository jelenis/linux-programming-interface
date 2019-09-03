#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <stdlib.h>
#include <limits.h>

static pthread_once_t once = PTHREAD_ONCE_INIT;
static pthread_key_t dirname_key; // key for dirname function
static pthread_key_t basename_key; // key for basename function

/**
 * Initializes keys for thread specific data, called once
 * useing pthread_once
 */
static void make_key() {
	pthread_key_create(&dirname_key, NULL);
	pthread_key_create(&basename_key, NULL);
}

/**
 * Thread safe implmentation of dirname(3).
 * Does not support wildcards and other special operators
 */
char* dirname(char* path) {
	char* buf;
		
	// init custom key for this function
	if (pthread_once(&once, make_key) != 0) 
		return NULL;
	
	// get specific data for this thread using custom key	
	buf = pthread_getspecific(dirname_key);

	/* check if there is any specifc data for this key, else it's the
	first time and buf needs to be allocated and set for the key */
	if (buf == NULL) {
		buf = malloc(PATH_MAX + 1);
		if (buf == NULL)
			return NULL;	
		
		// associate this key with buf 
		if (pthread_setspecific(dirname_key, buf) != 0)
			return NULL; 
	}	

	/* Process the path's string, the root slash should be left intact,
	 * while any trailing slashes should be ignored */
	char* last_slash = NULL;
	char* p = path;
	
	if (path[0] == '/' && path[1] != '/') {
		p ++; // skip leading slash
	}

	// loop through string, ignore the trailing slash
	for (p; *p != '\0'; p++) {
		// special case of double slash, return root
		if (*p == '/' && *(p+1) == '/') {
			buf[0] = '/';
			buf[1] = '\0';
			return buf;
		} else if (*p == '/' && *(p + 1) != '\0') {
			last_slash = p;
		}
	}
	
	// no slashes, means its local or the root	
	if (last_slash == NULL) {
		if (path[0] == '/') {
		       	// path is root
			buf[0] = '/';
			buf[1] = '\0';
			return buf;
		}
		// path is not root, must be local
		buf[0] = '.';
		buf[1] = '\0';
		return buf;
	}

	/* Reset pointer to start of the string and copy to buf until
	 the last slash */
	p = path; 
	for (p; p != last_slash; p++) {
		buf[p - path] = *p;	
	}

	buf[p - path] = '\0';
	return buf;
}

/**
 * Thread safe implmentation of basename(3).
 * Does not support wildcards and other special operators
*/
char* basename(char* path) {
	char* buf;
		
	// init custom key for this function
	if (pthread_once(&once, make_key) != 0) 
		return NULL;
	
	// get specific data for this thread using custom key	
	buf = pthread_getspecific(basename_key);

	/* check if there is any specifc data for this key, else it's the
	first time and buf needs to be allocated and set for the key */
	if (buf == NULL) {
		buf = malloc(PATH_MAX + 1);
		if (buf == NULL)
			return NULL;	
		
		// associate this key with buf 
		if (pthread_setspecific(basename_key, buf) != 0)
			return NULL; 
	}

	char* last_slash = NULL;
	char* p;
	char* start;
	/* Process basename string, ignores a trailing slash and should
	should copy all the chars past the end of the last slash into buf. 
	There is a special case if the path is just the root directory */
	if (path[0] == '/' && path[1] == '\0') { 
		// return root directory
		buf[0] = '/';
		buf[1] = '\0';
		return buf;
	}

	// loop through string, ignore the trailing slash
	for (char* p = path; *p != '\0'; p++) {

		// special case of double slash, return root
		if (*p == '/' && *(p+1) == '/') {
			buf[0] = '/';
			buf[1] = '\0';
			return buf;
		} else if (*p == '/' && *(p + 1) != '\0')
			last_slash = p;
	}

	if (last_slash == NULL) // no slash was found use entire path
		start = path;
	else
	      	start = last_slash + 1; // one char past the last slash

	// copy from the start (last slash) to the end of string into buf	
	for (p = start; *p != '\0'; p++) {
		if (*p == '/' && *(p + 1) == '\0') // ignore trailing slash
			break;
		buf[p - start] = *p;
	}
	buf[p - start] = '\0'; // null terminate

	return buf;
}

/**
 * Driver thread function for testing dirname & basename
 */
void* thread_func(void* arg) {
	char* path = (char*)arg;
	printf("dirname: %s basename: %s\n", dirname(path), basename(path));
}

/**
 * Driver to test dirname & basename
 */
int main(int argc, char* argv[]) {
	pthread_t thread1, thread2;
	pthread_create(&thread1, NULL, thread_func,  argv[1]);
	pthread_create(&thread2, NULL, thread_func,  argv[1]);
}
