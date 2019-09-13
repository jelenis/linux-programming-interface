#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/msg.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <errno.h>
#include <stddef.h>

#define KEY 0xaabbccdd

typedef struct {
	long mtype; // pid of reciever (1 for server)
	long from;  // pid of sender (1 for server)
	int seq;
} Message; 

#define MSG_SIZE (\
	 offsetof(Message, seq) \
	 - offsetof(Message, from) \
	 + sizeof(int)\
)

