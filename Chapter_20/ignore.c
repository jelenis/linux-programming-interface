#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>

void handler (int sig) {
	// unsafe, used for testing
	printf("handled\n");
}

int print_pending() {
	sigset_t pending;
	if (sigpending(&pending) == -1) {
		perror("sigpending");
		return -1;
	}
	for (int sig = 1; sig < NSIG; sig++) {
		if (sigismember(&pending, sig))
			printf("\t%s", strsignal(sig));
	}
	printf("\n");
}

int main(int argc, char* argv[]) {
	sigset_t block_set, prev_mask;	
	
	// first block sigint so it will be pending
	sigemptyset(&block_set);
	sigaddset(&block_set, SIGINT); // adds SIGINT to an empty mask
	if (sigprocmask(SIG_BLOCK, &block_set, &prev_mask) == -1) {
		perror("sigprocmask");
		return -1;
	}
	
	// wait, if a interrupt is signaled it will be pending
	sleep(3);
	
	
	// view current pending signals
	printf("printing pending signals\n");
	print_pending();

	// change the disposition to ingore
	if (signal(SIGINT, SIG_IGN) == SIG_ERR) {
		perror("signal");
		return -1;
	}	

	printf("printing pending signals after ignore\n");
	print_pending();

}

