#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

/**
 * thread that attempts to join with itself,
 * should fail on Linux
 */
static void* threadFunc(void* arg) {
	// join itsel (exit)
	if (pthread_join(pthread_self(), NULL))
		printf("call failed\n"); // not safe
}
/**
 * Driver program to test what happens a thread attempts to join with itself.
 * The thread should fail on Linux.
 */
int main() {
	pthread_t thread;

	int s = pthread_create(&thread, NULL, threadFunc, NULL);
	sleep(1);
	printf("main thread is still active\n");
	return 0;
}
