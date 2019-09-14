/**
 * Header file for simple chat client & server.
 * Similar to the talk(1) command.
 */
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

// amount of clients (needs to be large enough to hold uids)
#define CLIENTS 10000
// server message queue key 
#define KEY 0xaabbccdd 

// mtypes for the Message struct
#define M_CONNECT 1
#define M_DISCON 2
#define M_DATA 3

typedef struct {
	long mtype;
	int  msqid; 
	uid_t from;
	uid_t to;
	char data[1024];
} Message; 

/* System V message queues requires the size of all the data in a message
   excluding the first "long mtype" member. The offsetof() macro is used
   since there may be padding in the struct.  */
#define MSG_SIZE (\
	 offsetof(Message, data) \
	 + 1024 \
	 - offsetof(Message, msqid) \
)
