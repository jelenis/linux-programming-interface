#include <sys/socket.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <string.h>


/**
 * Demonstrates how the Kernel uses an ephermeral port number for
 * unbound listening sockets
 */
int main() {
	int lfd;
	struct sockaddr_in svaddr;
	memset(&svaddr, 0, sizeof(struct sockaddr_in));
	socklen_t len = sizeof(svaddr);	
	
	lfd = socket(AF_INET, SOCK_STREAM, 0);
	if (lfd == -1) {
		perror("socket");
		return -1;
	}
	
	// listen without binding
	if (listen(lfd, 10) == -1) {
		perror("listen");
		return -1;
	}


	// now display ephemeral port number
	if (getsockname(lfd, (struct sockaddr *) &svaddr, &len) == -1) {
		perror("getsockname");
		return -1;
	}
	printf("%d\n", svaddr.sin_port);

}
