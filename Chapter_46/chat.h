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
#include <fcntl.h>
#include <limits.h>
#include <pwd.h>

#define CLIENTS 10000

#define M_CONNECT 1
#define M_DISCON 2
#define M_DATA 3

#define KEY 0xaabbccdd 

typedef struct {
	long mtype;
	int  msqid; // id of client msgqueue
	uid_t from;
	uid_t to;
	char data[1024];
} Message; 


#define MSG_SIZE (\
	 offsetof(Message, data) \
	 + 1024 \
	 - offsetof(Message, msqid) \
)
