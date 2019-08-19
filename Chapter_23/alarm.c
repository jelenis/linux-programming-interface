#include <stdio.h>
#include <unistd.h>
#include <sys/time.h>
#include <signal.h>

/**
 * Implemented alarm using setitimer
 */
unsigned int alarm(unsigned int seconds) {
	struct itimerval timer;
	struct itimerval old;


	timer.it_value.tv_sec = (time_t) seconds;
	timer.it_value.tv_usec = 0;
	timer.it_interval.tv_sec = 0;			
	timer.it_interval.tv_usec = 0;
	
	if (setitimer(ITIMER_REAL, &timer, &old) == -1)
		return -1;

	return old.it_value.tv_sec;
}
/**
 * Part of driver program to test alarm, hanldes the SIGALRM
 * signal so that the process doesnt quit automatically
 * Uses unsafe stdio functions.
 */
void handler(int signo) {
	// unsafe stdio function
	printf("custom handler\n");
}

/**
 * Driver program to test alarm.
 * Sets the disposition for SIGALRM so that the process
 * doesnt quit and you can see that alarm(0) will disable pending
 * alarms.
 */
int main(int argc, char* argv[]) {
	struct sigaction sa;
	sa.sa_handler = handler;
	sigaction(SIGALRM, &sa, NULL);
	
	alarm(5);
	printf("returned: %d, now cancelling timer\n", alarm(0));
	sleep(5);
}

