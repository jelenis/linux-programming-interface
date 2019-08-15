#include <stdio.h>
#include <signal.h>

void abort() {
	sigset_t unblock;
	sigemptyset(&unblock);
	sigaddset(&unblock, SIGABRT);
	
	// ublock SIGABRT
	if (sigprocmask(SIG_UNBLOCK, &unblock, NULL) == -1) 
		return;

	raise(SIGABRT);

	/* here if the there is a handler that returned from SIGABRT
	   retore SIGABRT dispostion raise again to force abort */
	struct sigaction sa;
	sa.sa_handler = SIG_DFL;
	sa.sa_mask = unblock;
	sa.sa_flags = 0;
	sigaction(SIGABRT, &sa, NULL); 

	raise(SIGABRT);
}

/**
 * called by main to test abort
 */
void handler(int signo) {
	printf("handled\n");	
	return;
}

/**
 * Driver program to test abort
 */
int main(int argc, char* argv[]) {
	// change abort signal handler
	signal(SIGABRT, handler);
	abort();
	printf("should not be printed\n");
}
