#include <stdio.h>
#include <unistd.h>
#include <signal.h>

/**
 * Simple implementation of the siginterrupt
 */
int siginterrupt(int sig, int flag) {
	struct sigaction sa;

	// retrieve current sigaction
	if (sigaction(sig, NULL, &sa) == -1)
		return -1;

	if (flag) 
		sa.sa_flags &= ~SA_RESTART;
	else 
		sa.sa_flags |= SA_RESTART;

	// apply new sigaction
	return sigaction(sig, &sa, NULL);
}

/**
 * Driver program for siginterrupt
 */
int main(int argc, char* argv[]) {
	siginterrupt(SIGINT, 1);

	return 0;
}
