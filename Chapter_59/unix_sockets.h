#include <sys/socket.h>
#include <sys/un.h>
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <limits.h>

#define SV_SOCKET "\0/svr"
#define BUF_SIZE 10

/**
 * Creates a unix domain socket and binds an address to path
 */
int unBind(const char* path, int type);


/**
 * Creates a unix domain socket then binds and listens to the socket 
 * located at path
 */
int unListen(const char *path, int backlog);


/**
 *  Creates a socket and connects it to the server at path
 */
int unConnect(const char* path, int type);

