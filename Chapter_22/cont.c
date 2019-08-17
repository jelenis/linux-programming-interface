#include <stdio.h>
#include <signal.h>

void handler(int signo) {
	// unsafe, just for demo
	printf("handler\n");
}

/**
 * Demonstrates that custom handlers for SIGCONT are deferred until SIGCONT is
 * unblocked, even though the signal will still wake the process immediatley
 */
int main(int argc, char* argv[]){
	sigset_t block_set, prev_set;
	struct sigaction sa;
	
	// block SIGCONT
	sigemptyset(&block_set);
	sigaddset(&block_set, SIGCONT);
	sigprocmask(SIG_BLOCK, &block_set, &prev_set);

	// create handler for SIGCONT	
	sa.sa_handler = handler;
	sigaction(SIGCONT, &sa, NULL);

	printf("stopping...\n");
	raise(SIGSTOP);
	printf("process resumed by SIGCONT\n");
	
	// unblock SINGCONT, this should trigger handler
	sigprocmask(SIG_SETMASK, &prev_set, NULL);	
}
