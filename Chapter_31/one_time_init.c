#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

/* Define structure with flag check if first time and mutex */
typedef struct one_time_control {
	int flag;
	pthread_mutex_t mtx; 
} one_time_control;

/* Provide initialization macro (analog to PTHREAD_ONCE_INIT) */
#define ONCE_INIT {.flag = 0, .mtx = PTHREAD_MUTEX_INITIALIZER}

/**
 * Custom implmentation of pthread_once(3), see man page for details
 */
int one_time_init(one_time_control *control, void (*init_routine) (void)) {
	// aquire controls mutex
	if (pthread_mutex_lock(&control->mtx) != 0)
		return -1;
	if (control->flag == 1) // return is not first time
		return -1;
	control->flag = 1; 

	init_routine();

	if (pthread_mutex_unlock(&control->mtx) != 0)
		return -1;
}

/**
 * Driver code to test implementation
 */
static one_time_control once = ONCE_INIT;

void runs_once() {
	printf("THIS IS PRINTED ONCE\n");
}
void* thread_func(void* arg) {
	one_time_init(&once, runs_once);	
}
int main() {
	pthread_t thread_A;

	pthread_create(&thread_A, NULL, thread_func, NULL);
	pthread_join(thread_A, NULL);
	one_time_init(&once, runs_once);	
}
