#include <stdio.h>
#include <fcntl.h>
#include <mqueue.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <limits.h>
#include <sys/wait.h>
#include <signal.h>
#include <sys/stat.h>

#define SERVER "/chat"
#define BUFFER_SIZE 256
#define DATA 0
#define END 1

typedef struct {
	char to[NAME_MAX];
	char from[PATH_MAX];
	char data[BUFFER_SIZE];
} Message;

int perms = S_IRUSR | S_IWUSR | S_IWOTH | S_IROTH | S_IWGRP | S_IRGRP;
