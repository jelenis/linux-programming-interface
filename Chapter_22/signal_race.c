#include <stdio.h>
#include <signal.h>
#include <limits.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>

/**
 * Signal handler, uses unsafe stdio function for demo purposes 
 */
void handler(int signo, siginfo_t *si, void* ucontext_t) {
	printf("recieved signal %d", signo);
	printf("type: %s\n", si->si_code == SI_QUEUE ? "real-time" : "other");
}

/**
 * Demonstrates the linux behaivour of recieving real-time signals after
 * standard ones
 */
int main(int argc, char* argv[]) {
	sigset_t blocked, prev;
	struct sigaction usr_act, rt_act;
	union sigval sv;

	/* block SIGUSR1 and SIGRTMIN, just assume no errors */
	sigemptyset(&blocked);
	sigaddset(&blocked, SIGUSR1);
	sigaddset(&blocked, SIGRTMIN);
	sigprocmask(SIG_BLOCK, &blocked, &prev);
	
	usr_act.sa_sigaction = handler;	
	usr_act.sa_flags = SA_SIGINFO;
	sigfillset(&usr_act.sa_mask);
	sigaction(SIGUSR1, &usr_act, NULL);

	rt_act.sa_sigaction = handler;	
	rt_act.sa_flags = SA_SIGINFO;
	sigfillset(&rt_act.sa_mask);
	sigaction(SIGRTMIN, &rt_act, NULL);
	
	time_t t = time(NULL);
	if (t == -1) {
		perror("time");
		return -1;
	}	
	srandom(t);

	// make both signals pending in a random order
	if (random()%2) {
		raise(SIGUSR1);
		sigqueue(getpid(), SIGRTMIN, sv);
	} else {
		sigqueue(getpid(), SIGRTMIN, sv);
		raise(SIGUSR1);
	}

	printf("unblocking\n");
	// unblock signals
	sigprocmask(SIG_SETMASK, &prev, NULL);
	
	// the user signal is recieved first, and real-time after
	return 0;
}
