#include <stdio.h>
#include <time.h>
#include <signal.h>
#include <unistd.h>

/**
 * SIGALRM handler, uses unsafe stdio for demo
 */
void handler(int signo) {
	// unsafe printf
	printf("handler\n");
}

/**
 * Demonstrates the behaviour of timer_create with a null event
 */
int main(int argc, char* argv[]) {
	timer_t timerid;
	timer_t timerid2;
	struct sigevent evp;
	struct sigaction sa;
	struct itimerspec ts;
	sa.sa_flags = 0;
	sa.sa_handler = handler;

	if (sigaction(SIGALRM, &sa, NULL) == -1) {
		perror("sigaction");
		return -1;
	}
	// capture timer event throught the signal handler
	evp.sigev_notify = SIGEV_SIGNAL;
	evp.sigev_signo = SIGALRM; // send SIGALRM
	evp.sigev_value.sival_ptr = timerid; // send the timer id
	if (timer_create(CLOCK_REALTIME, &evp, &timerid) == -1) {
		perror("timer_create");
		return -1;

	}

	// capture timer event throught the same signal handler
	if (timer_create(CLOCK_REALTIME, NULL, &timerid2) == -1) {
		perror("timer_create");
		return -1;
	}

	// start both timers 
	ts.it_value.tv_sec = 5;
	ts.it_value.tv_nsec = 0;
	ts.it_interval.tv_sec = 0;
	ts.it_interval.tv_nsec = 0;
	if (timer_settime(timerid, 0, &ts, NULL) == -1) {
		perror("timer_settime");
		return -1;
	}

	struct itimerspec ts2 = ts;
	ts2.it_value.tv_sec = 10;
	if (timer_settime(timerid2, 0, &ts, NULL) == -1) {
		perror("timer_settime");
		return -1;
	}

	// the same handler should be invoked for both alarms
	sleep(15);
	return 0;
}


