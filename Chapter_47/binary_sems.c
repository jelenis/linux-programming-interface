/* 
 * Implement a binary semaphore protocol using named pipes (fifo streams).
 */
#include <sys/types.h>
#include <sys/sem.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include "semun.h"                      /* Definition of semun union */

int reserveSemNB(char* path) {
	char buf;
	int fd = open(path, O_CREAT  | O_APPEND | O_NONBLOCK,  O_RDONLY);
	if (fd == -1)
		return -1;
	if (read(fd, &buf, 1) != 1)
		return -1;
	close(fd);
	return 0;
}


int initSemAvailable(char* path)
{
	int fd;
	if (mkfifo(path, S_IRUSR | S_IWUSR) == -1)
		return -1;
	fd = open(path, O_CREAT,  O_WRONLY);
	if (fd == -1)
		return -1;
	if (write(fd, &"1", 1) != 1)
		return -1;
	close(fd);
	return 0;
}

int initSemInUse(char* path)
{
	if (mkfifo(path, S_IRUSR | S_IWUSR | S_IWGRP) == -1)
		return -1;
	return 0;
}

int reserveSem(char* path)
{
	char buf;
	int fd = open(path, O_RDONLY);
	if (fd == -1)
		return -1;
	if (read(fd, &buf, 1) != 1)
		return -1;
	close(fd);
	return 0;
}

int releaseSem(char* path)
{
	int fd = open(path, O_APPEND | O_NONBLOCK |  O_WRONLY);
	if (fd == -1)
		return -1;
	printf("write is blocking\n");
	if (write(fd, &"1", 1) != 1)
		return -1;
	close(fd);
	return 0;
}


/**
 * Driver program to test semaphore API
 */
char* path;
static void handler() {
	printf("attempting to release semaphore\n");
	if (releaseSem(path) == -1) {
		perror("releaseSem");
		exit(EXIT_FAILURE);
	}
	printf("released\n");
}

int main(int argc, char* argv[]) {
	unlink(path);
	if (argc < 2) {
		printf("Please provide a semahpore path\n");
		return -1;
	}
	path = argv[1];

	// init sem
	if (initSemInUse(path) == -1) {
		perror("initSemInUse");
		return -1;
	}
	switch(fork()) {
		case -1:
			return -1;	
		case 0:
			sleep(3);
			handler();
			_exit(EXIT_SUCCESS);
		default:
			break;

	}
	printf("about to wait on event\n");
	if (reserveSem(path) == -1) {
		perror("waitForEventFlag");
		return -1;	
	}
	printf("done waiting for event\n");

}
