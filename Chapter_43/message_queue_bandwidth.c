#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wait.h>
#include <time.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/types.h>
#include <errno.h>

#define MESSAGE_SIZE 100
struct msgbuf  {
	long mtype;
	char mtext[MESSAGE_SIZE];
};


static void send(int num, char* block) {
	struct msgbuf msg;
	msg.mtype = 1;	 // 1 for send

	for (int i = 0; i < num; i ++)
		if (msgsnd(1234, (void*) &msg, sizeof(msg.mtext), IPC_NOWAIT) == -1) {
			perror("msgsnd");
			exit(EXIT_FAILURE);
		}

}
static void recv(int num) {
	struct msgbuf msg;
	// return if there are no messages to read
	
	for (int i = 0; i < num; i++)	
		if (msgrcv(1234, (void*) &msg, sizeof(msg.mtext),
					0, MSG_NOERROR | IPC_NOWAIT) == -1) {

			if (errno != ENOMSG) {
				perror("msgrcv");
				exit(EXIT_FAILURE);
			}
		}
}

/**
 * Compares the speed of message queues, witch is signifigantly slower than
 * byte streams
 */
int main(int argc, char* argv[]) {
	int fildes[2];	
	char* block;
	int num;
	ssize_t size = MESSAGE_SIZE;

	if (argc < 2) {
		printf("Enter number of messages to send");
		exit(EXIT_FAILURE);
	}	

	num = strtol(argv[1], NULL, 10);
	block = malloc(size);
	
	int qid = msgget(1234, IPC_CREAT | 0666);
	if (qid  == -1) {
		perror("msgget");
		exit(EXIT_FAILURE);
	}

	char* buf = malloc(sizeof(block));

		
	struct timespec start, end;
	clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &start);
		
	switch (fork()) {
		case -1:
			break;
		case 0: // child reads
			recv(num);
			_exit(EXIT_SUCCESS);
			
		default: // parent writes
			send(num, block);
			wait(NULL);
			clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &end);
			double res = (end.tv_sec - start.tv_sec) * 1e3 
				+ (end.tv_nsec - start.tv_nsec) / 1e6; 	

			printf("time: %lf ms\t bytes: %ld\tKb/s: %lf\n", res,
				       	size*num, size*num/res/1e6);
			break;
	}

}
