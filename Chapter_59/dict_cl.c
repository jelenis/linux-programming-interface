/**
 * Simple client to communicate with dictionary server.
 * Uses localhost and port 50000 these are hard coded for brevity.
 * Usage:
 *        a:  add space delimited key/value pair
 *        g:  get value of given key 
 *        d:  delete key/value pair by key
 *
 * ex: a greet hi         # add
 *     g greet 		  # get
 *     	 'hi'
 *     a greet hello      # modify
 *     g greet
 *     	 'hello'         
 *     d greet            # delete
 *     g greet
 *     	 'null'
 */
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main() {
	struct addrinfo hints;
	struct addrinfo *result, *rp;
	char buf[256];
	int sfd;	

	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_family = AF_UNSPEC; // ip4 and ip6
	hints.ai_socktype = SOCK_STREAM; // TCP

	if (getaddrinfo("localhost", "50000", &hints, &result) == -1) {
		perror("getaddrinfo");
		return -1;
	}

	/* geaddrinfo returns a linked list of possible address structures
	   try each one until we connect */
	for (rp = result; rp != NULL; rp = rp->ai_next) {
		sfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
		if (sfd == -1)
			continue; // try next address

		// exit loop if connection is successful
		if (connect(sfd, rp->ai_addr, rp->ai_addrlen) != -1)
			break;

		close(sfd);
	}
	freeaddrinfo(result);

	while (fgets(buf, sizeof(buf), stdin)) {
		size_t len = strlen(buf); 
		// skip empty input 
		if (len == 1)
			continue;
		if (write(sfd, buf, len) != len) {
			perror("write");
			return -1;
		}

		// sent a get request, wait for response
		if (buf[0] == 'g') {
			if (read(sfd, buf, sizeof(buf)) > 0)  {
				buf[sizeof(buf)- 1] = '\0';
				printf("%s", buf);
			}
		}
	}



}
