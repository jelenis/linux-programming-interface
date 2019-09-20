#include <stdio.h>
#include <fcntl.h>
#include <mqueue.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <limits.h>

#define SERVER "/sq"

typedef struct {
	char cq[NAME_MAX];
	int seq;
} Request;
