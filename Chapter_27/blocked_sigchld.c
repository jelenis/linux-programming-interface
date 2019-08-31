#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/wait.h>


static void handler (int signo) {
	int status;
	if (wait(&status) == -1) 
		perror("wait"); // unsafe
	
}

/**
 * Demonstrates the effects of blocking an established SIGCHLD handler, then
 * having a child exit
 */
int main() {
	struct sigaction sa;
	
	// establish handler
	sa.sa_handler = handler;
	sigaction(SIGCHLD, &sa, NULL);
	
	// block SIGCHLD
	sigset_t blocked;
	sigemptyset(&blocked);
	sigaddset(&blocked, SIGCHLD);
	sigprocmask(SIG_BLOCK, &blocked, NULL);

	switch(fork()) {
		case 0: // child exits immediatley
			printf("child exiting\n");
			_exit(EXIT_SUCCESS);
			break;
		default: // parent
			break;
	}

	sleep(3); // let child exit

	// ublock SIGCHLD
	sigprocmask(SIG_UNBLOCK, &blocked, NULL);

	return 0;
}
