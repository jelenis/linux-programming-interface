#include <stdio.h>
#include <signal.h>
#include <unistd.h>

#define SIG_HOLD    (void (*)(int))5

/**
 * From the System V API, implemented using the modern API
 * Refer to SIGSET(3) for details
 */
typedef void (*sighandler_t)(int);
void  (*sigset(int sig, sighandler_t disp)) {
	struct sigaction sa, prev_sa;
	sigset_t prev_set;

	sa.sa_flags = 0;	
	if (sigemptyset(&sa.sa_mask) == -1)
		return (void*)-1;

	// get blocked signals
	if (sigprocmask(0, NULL, &prev_set) == -1)
		return (void*)-1;

	if (disp != SIG_HOLD) {
		sigset_t unblocked;

		sa.sa_handler = disp;
		
		// unblock signal
		if (sigemptyset(&unblocked) == -1)
			return (void*)-1;
		if (sigaddset(&unblocked, sig) == -1)
			return (void*)-1;
		if (sigprocmask(SIG_UNBLOCK, &unblocked, NULL) == -1)
			return (void*)-1;
		if (sigaction(sig, &sa, NULL) == -1)
			return (void*)-1;
	} else {
		sigset_t blocked;
		
		// block signal	
		if (sigemptyset(&blocked) == -1)
			return (void*)-1;
		if (sigaddset(&blocked, sig) == -1)
			return (void*)-1;
		if (sigprocmask(SIG_BLOCK, &blocked, NULL) == -1)
			return (void*)-1;
	}
	
	// return previous set, or SIG_HOLD if blocked
	if (sigismember(&prev_set, sig))
		return SIG_HOLD;
	
	// get previous handler
	if (sigaction(sig, NULL, &prev_sa) == -1)
		return (void*)-1;
	return prev_sa.sa_handler;
}

/**
 * Adds a signal to the process mask
 * From the System V API, implemented using the modern API
 * See SIGSET(3) for details
 */
int sighold(int sig) {
	sigset_t blocked;

	if (sigemptyset(&blocked) == -1)
		return -1;
	if (sigaddset(&blocked, sig) == -1)
		return -1;
	if (sigprocmask(SIG_BLOCK, &blocked, NULL) == -1)
		return -1;

	return 0;
}

/**
 * Deletes a signal from the process mask
 * From the System V API, implemented using the modern API
 * See SIGSET(3) for details
 */
int sigrelse(int sig) {
	sigset_t unblocked;

	if (sigemptyset(&unblocked) == -1)
		return -1;
	if (sigaddset(&unblocked, sig) == -1)
		return -1;
	if (sigprocmask(SIG_UNBLOCK, &unblocked, NULL) == -1)
		return -1;

	return 0;
}

/**
 * Ignores a signal 
 * From the System V API, implemented using the modern API
 * See SIGSET(3) for details
 */
int sigignore(int sig) {
	if (signal(sig, SIG_IGN) == SIG_ERR)
		return -1;
	return 0;
}

/**
 * Waites for a signal
 * From the System V API, implemented using the modern API
 * See SIGPAUSE(3) for details
 */
int sigpause(int sig) {
	return pause();
}

