#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <errno.h>

#define SOCK1 "/tmp/a"
#define SOCK2 "/tmp/b"
#define SOCK3 "/tmp/c"

/**
 * Demonstrates that sending a datagram to a connected UNIX socket that
 * isn't from the connected peer will result in a permission error
 */
int main () {
	int fd1, fd2, fd3, req, resp;
	struct sockaddr_un a1, a2, a3;	

	fd1 = socket(AF_UNIX, SOCK_DGRAM, 0);
	if (fd1 == -1) {
		perror("socket");
		return -1;
	}
	fd2 = socket(AF_UNIX, SOCK_DGRAM, 0);
	if (fd1 == -1) {
		perror("socket");
		return -1;
	}
	fd3 = socket(AF_UNIX, SOCK_DGRAM, 0);
	if (fd1 == -1) {
		perror("socket");
		return -1;
	}

	if (remove(SOCK1) == -1 && errno != ENOENT) {
		perror("socket");
		return -1;
	}
	if (remove(SOCK2) == -1 && errno != ENOENT) {
		perror("socket");
		return -1;
	}
	if (remove(SOCK3) == -1 && errno != ENOENT) {
		perror("socket");
		return -1;
	}

	memset(&a1, 0, sizeof (struct sockaddr_un));
	memset(&a2, 0, sizeof (struct sockaddr_un));
	memset(&a3, 0, sizeof (struct sockaddr_un));

	a1.sun_family = AF_UNIX;
	a2.sun_family = AF_UNIX;
	a3.sun_family = AF_UNIX;

	strncpy(a1.sun_path, SOCK1, sizeof(a1.sun_path));
	strncpy(a2.sun_path, SOCK2, sizeof(a2.sun_path));
	strncpy(a3.sun_path, SOCK3, sizeof(a3.sun_path));

	if (bind(fd1, (struct sockaddr *) &a1, sizeof(struct sockaddr_un)) == -1) {
		perror("bind");
		return -1;
	}
	if (bind(fd2, (struct sockaddr *) &a2, sizeof(struct sockaddr_un)) == -1) {
		perror("bind");
		return -1;
	}
	if (bind(fd3, (struct sockaddr *) &a3, sizeof(struct sockaddr_un)) == -1) {
		perror("bind");
		return -1;
	}

	// connect a to b
	if (connect(fd1, (struct sockaddr *) &a2, sizeof(struct sockaddr_un)) == -1) {
		perror("connect");
		return -1;
	}
		
	//send a datagram to a from c
	req = 1;
	if (sendto(fd3, &req, sizeof(int), 0, (struct sockaddr *) &a1, sizeof(struct sockaddr_un)) == -1) {
		perror("sendto");
		return -1;
	}
	
	// send a datagram to a from b
	req = 2;
	if (sendto(fd2, &req, sizeof(int), 0, (struct sockaddr *) &a1, sizeof(struct sockaddr_un)) == -1) {
		perror("sendto");
		return -1;
	}

	// atemp to read from a
	if (read(fd1, &resp, sizeof(int)) < 0) {
		perror("read");
		return -1;
	}	

	// will print 2 (from connected client)
	printf("%d\n", resp);
	

}
