#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>


void handler(int sig) {
	printf("handler\n");
}

/**
 * Demonstrates the use of SA_NODEFER and SA_RESETHAND in the 
 * sigaction call
 */
int main(int argc, char* argv[]) {
	sigset_t block_set, prev_mask;	
	struct sigaction sa;

	sa.sa_handler = handler;
	sigemptyset(&sa.sa_mask);
	sigaddset(&sa.sa_mask, SIGINT);	

	// reset the handler to default and dont defer parallel signals 
	sa.sa_flags = SA_RESETHAND | SA_NODEFER;

	if (sigaction(SIGINT, &sa, NULL) == -1) {
		perror("sigaction");
		return -1;
	}
	
	printf("The first SIGINT will be handled,"
		"this will cause the handler to be reset to its"
	       	"default function thus the second will instantly"
	       	"close the program\n");
	/* have to continually sleep because the first SIGINT will stop 
	   the sleep call and the program will end before a second SIGINT will 
	   can even be sent */
	while (1) sleep(1);	
}

