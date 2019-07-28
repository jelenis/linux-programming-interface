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
	int opt;
	
	if (argc < 2) {
		printf("Usage: tail [-n num] file\n");
		return -1;
	}

	while ((opt = getopt(argc, argv, "n:")) != -1) {
		if (opt == 'n') {
			offset = atoi(optarg);
			if (offset > 100) offset = 100;
			else if (offset == 0) return 0;
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
	while((bytes_read = read(fd, buf, length_to_read)) != -1) {
		ssize_t i = bytes_read -1 ;
		
		// count lines in reverse
		for(i; i >= 0; i--) {
			if (buf[i] == '\n') linecnt++;
			if (linecnt >= offset + 1) break;
		}

		// the length starting from the end of the string to after
		// the lastest newline. This is usually somewhere inside buf
		ssize_t len = bytes_read - i - 1;

		// allocate 1 extra char for null
		// append the line to the end of the the buffer list
		buffers[buffcnt] = malloc((len+1)*sizeof(char));

		// there is no newline in buf, use strn to copy the correct length
		// copying starts from the 1 after the last newline found
		strncpy(buffers[buffcnt], buf + i + 1, len);
		buffers[buffcnt][len] = '\0'; 
		buffcnt++;

		// linecnt should be 1 greater than number of newlines
		// eg: a 7 line file only has 6 newlines
		// also exit if the current fd offset is at 0
		if (linecnt >= offset + 1 || cur == 0) break;
		
		// move fd pointer back 1 BUFFER size and make sure this will is 
		// not further than the begining of the buffer 
		if (cur - BUFFER - bytes_read >= 0) {
			cur = lseek(fd, -BUFFER - bytes_read, SEEK_CUR);
		} else {
			length_to_read = lseek(fd, 0, SEEK_CUR) - BUFFER;
			cur = lseek(fd, 0, SEEK_SET); 
		}
	}

	close(fd);
	for (ssize_t i = buffcnt - 1; i >= 0; i--){
		printf("%s", buffers[i]);
		free(buffers[i]);
	}

	//printf("\n");
	return 0;
}
