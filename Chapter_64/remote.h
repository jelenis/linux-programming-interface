#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <signal.h>
#include <netdb.h>
#include <errno.h>
#include <sys/epoll.h>
#include "inet_sockets.h"
#include "tty_functions.h"
#include "pty_fork.h"

#define SERVICE "50000"
#define BACKLOG 10
#define MAX_NAME 1000

