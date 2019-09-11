#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <wait.h>

#define SRV_SIZE 10 // use fixed message lengths
#define CLI_SIZE SRV_SIZE

/**
 * converts string pointed to by str to upperase ASCII characters
 */
static void toUpper(char* str, ssize_t nbytes) {
	for (ssize_t i = 0; i < nbytes; i++) {
		if (str[i] >= 'a' && str[i] <= 'z')
		       	str[i] -= 32;
	}
}

/**
 * Uses a bidirectional pipes to communicate between parent and child to
 * convert string to uppercase characters
 * NOTE: the client is the child, however it should be the server and vice versa
 */
int main(int argc, char* argv[]) { 
	int to_client[2], to_server[2];	
	char server_buf[SRV_SIZE + 1]; // +1 for null terminator
	char client_buf[CLI_SIZE];
	ssize_t nbytes;

	// create two pipes
	if (pipe(to_client) == -1) {
		perror("pipe");
		exit(EXIT_FAILURE);
	}
	if (pipe(to_server) == -1) {
		perror("pipe");
		exit(EXIT_FAILURE);
	}

	switch(fork()) {
		case -1:
			exit(EXIT_FAILURE);
		
		case 0:
			close(to_client[1]); // close write end
			close(to_server[0]); // close read end
			
			// block until parent sends  text to capitilize
			while ((nbytes = read(to_client[0], client_buf, CLI_SIZE)) > 0){
				toUpper(client_buf, nbytes);
				if (write(to_server[1], client_buf, nbytes) != nbytes) {
					perror("write to server");
					_exit(EXIT_FAILURE);
				}	
			}
			_exit(EXIT_SUCCESS);
			break;

		default:
			close(to_client[0]); // close read end
			close(to_server[1]); // close write end

			// read block from stdin while input is larger than 1 byte (newline)
			while ((nbytes = read(STDIN_FILENO, server_buf, SRV_SIZE)) > 0){
				if (write(to_client[1], server_buf, nbytes) != nbytes) {
					perror("write to client");
					exit(EXIT_FAILURE);
				}	
				
				// block until client is finished
				nbytes = read(to_server[0], server_buf, SRV_SIZE);
				if (nbytes == 0){
					perror("read from client");
					exit(EXIT_FAILURE);
				}
				// server buf contains 1 extra byte for the newline character
				server_buf[nbytes] = '\0';
				printf("%s", server_buf);
				fflush(stdout);
			}
			exit(EXIT_SUCCESS);	

	}

}
