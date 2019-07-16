#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#define MODE 1


struct iovec {
	void* iov_base;
	int iov_len;
};


int readv(int fd, const struct iovec* iov, int iocnt);
int writev(int fd, const struct iovec *iov, int iocnt);

// main driver program to test implementation of writve and readv in sys/uio.h
int main() {
	int fd = open("test.txt", O_RDWR | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);
	int iocnt = 3;
	struct iovec* iov = malloc(sizeof(struct iovec)*iocnt);
	char* str = "_hello";
	

	for (int i=0; i< iocnt; i ++) {
		iov[i].iov_len = strlen(str)+1;

		// if read mode (1) allocate space for string, else point to it
		if (MODE == 1) iov[i].iov_base = malloc(iov[i].iov_len);
		else iov[i].iov_base = str;
	}
	
	// read vector
	if (MODE == 1) {
		if (readv(fd, iov, iocnt) == -1) {
			printf("Error reading\n");
		}

		for (int i=0; i< iocnt; i++) {
			printf("%s\n", iov[i].iov_base);
			free(iov[i].iov_base);
		}
	// write to file
	} else writev(fd, iov, iocnt);
	

	free(iov);
	return 0;
}


int readv(int fd, const struct iovec* iov, int iocnt){
	int total_read = 0;
	
	for (int i = 0; i < iocnt; i++) {
		int num_read = read(fd, iov[i].iov_base, iov[i].iov_len);
		if (num_read == -1) return -1;
		total_read += num_read;
	}
	return total_read;
}

int writev(int fd, const struct iovec *iov, int iocnt) {
	int total_written = 0;

	//int* buf = malloc(iovec[i]->iov_len);
	for (int i = 0; i < iocnt; i++) {
		int num_written = write(fd, iov[i].iov_base, iov[i].iov_len);	
		if (num_written == -1) return -1;
		total_written += num_written;
	}
	return total_written;
}
