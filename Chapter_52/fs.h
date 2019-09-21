#include <stdio.h>
#include <fcntl.h>
#include <mqueue.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <limits.h>

#define SERVER "/fs"
#define BUFFER_SIZE 1
#define DATA 0
#define END 1

typedef struct {
	int type;
 	char buf[BUFFER_SIZE];
} Response;
typedef struct {
	char cq[NAME_MAX];
	char file[PATH_MAX];
} Request;


