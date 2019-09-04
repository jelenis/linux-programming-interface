#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>

/**
 * Demonstrates that each thread has different pending signals
 */
void* thread_func(void* arg) {
	sleep(1); // wait for signals
	sigset_t pending;
	sigpending(&pending);
	if (sigismember(&pending, SIGUSR1)) {
		printf("thread %ld got SIGUSR1\n", pthread_self());
	}
	if (sigismember(&pending, SIGUSR2)) {
		printf("thread %ld got SIGUSR2\n", pthread_self());
	}
}

int main(int argc, char* argv[]) {
	pthread_t thread_A, thread_B;	
	sigset_t blocked;
	int unblock = 1;
	int block = 0;
	
	// block user signals in main thread, other threads will inherit
	sigemptyset(&blocked);
	sigaddset(&blocked, SIGUSR1);
	sigaddset(&blocked, SIGUSR2);
	sigprocmask(SIG_BLOCK, &blocked, NULL);

	pthread_create(&thread_A, NULL, thread_func, &unblock);
	pthread_create(&thread_B, NULL, thread_func, &block);

	// send seperate signals to each thread
	pthread_kill(thread_A, SIGUSR1);
	pthread_kill(thread_B, SIGUSR2);

	pthread_join(thread_A, NULL);
	pthread_join(thread_B, NULL);
}
