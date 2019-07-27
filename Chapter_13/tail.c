#include<stdio.h>
#include<string.h>
#include<unistd.h>
#include<stdlib.h>
#include<errno.h>
#include<fcntl.h>

#ifndef BUFFER
#define BUFFER 1024
#endif
int main(int argc, char* argv[]) {
	int offset = 10;	
	int opt = 0;
	
	if (argc < 2) {
		printf("Usage: tail [-n num] file\n");
		return -1;
	}

	while ( (opt = getopt(argc, argv, "n:")) != -1) {
		if (opt == 'n') {
			offset = atoi(optarg);
			if (offset > 100) offset = 100;
			break;
		}
	}
		
	char* filename = argv[argc-1];
	int fd = open(filename, O_RDONLY);
	if (fd == -1) {
		printf("%s %s\n", filename, strerror(errno));
		return -1;
	}

	
	char buf[BUFFER];
	char* buffers[100];
        size_t buffcnt = 0;		
	ssize_t length_to_read = BUFFER;
	ssize_t bytes_read;
	int linecnt = 0;
	char* p = 0;
		
	int cur = lseek(fd, -BUFFER, SEEK_END);
		printf("SEEK at : %d\n", lseek(fd, 0, SEEK_CUR));
	
	while((bytes_read = read(fd, buf, length_to_read)) != -1) {
		ssize_t i = bytes_read;
		for(i; i >= 0; i--) {
			if (buf[i] == '\n') linecnt++;
			if (linecnt == offset) break;
		}
		ssize_t len = bytes_read - i - 1;
		printf("allocating: %d\n", len + 1);
		buffers[buffcnt] = malloc((len+1)*sizeof(char));
		strncpy(buffers[buffcnt], buf, len);
		buffers[buffcnt][len] = '\0';
		printf("created %s", buffers[buffcnt]);
		buffcnt++;


		if (linecnt == offset || cur == 0) {
			break;
		}

		if (cur - BUFFER - bytes_read >= 0) {
			cur = lseek(fd, -BUFFER-bytes_read-1, SEEK_CUR);
		} else {
			length_to_read = lseek(fd, 0, SEEK_CUR) - BUFFER;
			cur = lseek(fd, 0, SEEK_SET); 
			printf("length to read %lu\n", length_to_read);
		}
		printf("current SEEK = %d\n", cur);
	}
	printf("line count: %d\n", linecnt);	
	close(fd);
	for (ssize_t  i = buffcnt - 1; i >= 0; i--){
		printf("%s", buffers[i]);
		free(buffers[i]);
	}

	//printf("\n");
	return 0;
}
