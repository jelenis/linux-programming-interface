#define _XOPEN_SOURCE 500
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ftw.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/inotify.h>
#include <limits.h>

#define BUF_LEN (sizeof(struct inotify_event) + NAME_MAX + 1)
int inotifyFd;

/**
 * Called recursivley by nftw to add file event watchers using the nftw API 
 * skips all files but directories
 */
int listen(const char* fpath, const struct stat* sb, int type, struct FTW* ftwbuf) {
	int wd;
	
	// skip anything that isnt a directory
	if (type !=FTW_D) 
		return 0;

	wd = inotify_add_watch(inotifyFd, fpath, IN_ALL_EVENTS);	

	if (wd != -1) {
		printf("watching %s\n", fpath);
	}

	return 0;
}

/**
 * prints just a few of the events specified in the mask
 */
int print_event(struct inotify_event* i) {
	printf("cookie %4d ", i->cookie);
	if (i->mask & IN_DELETE) printf("deleted");
	if (i->mask & IN_CREATE) printf("created");
	if (i->mask & IN_MODIFY) printf("modified");
	if (i->len > 0)
		printf(" name = %s", i->name);	
	printf("\n");
}

/**
 * recursivley walks the file tree re-adding inotify watchers
 * returns 0 on success, -1 on error
 */
int add_listeners(int argc, char* argv[]) {
	for (int i = 1; i < argc; i++) {
		if (nftw(argv[i], listen, 20, 0) == -1)
			return -1;
	}

	return 0;
}


/**
 * Recursively watch directories specfied by the command line arguments
 */
int main(int argc, char* argv[]) {
	char buf[BUF_LEN];
	struct inotify_event* event;
	ssize_t num_read;

	if (argc < 2) {
		printf("missing argument\n");
		return -1;
	}

	inotifyFd = inotify_init();
	if (inotifyFd == -1) {
		perror("inotify_init");
		return -1;
	}

	// recursively add file events
	if (add_listeners(argc, argv) == -1) {
		perror("add_listeners");
		return -1;
	}

	while (1) {
		num_read = read(inotifyFd, buf, BUF_LEN);
		if (num_read <= 0) {
			perror("read");
			return -1;
		}

		/* iterate through buffer containing events, there may be padding in the
		 * event struct so use a char to point to individual bytes */
		for (char* p = buf; p < buf + num_read;) {
			// read the event starting at the byte pointed to by p 
			event = (struct inotify_event *) p;
			print_event(event);
			if (event->mask & IN_ISDIR  && event->mask & IN_CREATE) {
				add_listeners(argc, argv);
				// recursively add file events
				if (add_listeners(argc, argv) == -1)
					perror("add_listeners");
			}

			// move the pointer 1 struct length forward
			p += sizeof(struct inotify_event) + event->len;
		}		

	}
	return 0;
}
