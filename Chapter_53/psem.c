#include <stdio.h>
#include <stdarg.h>
#include <fcntl.h>
#include "semun.h"
#include <string.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/stat.h>
#include <errno.h>
#include <limits.h>

typedef int sem_t;
typedef struct {
	int semid;
	int cnt;
	int unlink;
	char filename[NAME_MAX];
} Semobj;

#define SEMS 10000
static Semobj semidv[SEMS]; 
static int sem_ind = 0;
static int init = 0;
#define SEM_FAILED ((sem_t*)-1)


/**
 * Wrapper for System V sempahores to implement the POSIX semaphore API.
 * This is just a subset of the real functionality see man page for more details.
 */
sem_t *sem_open(const char *name, int oflag, ...) {
	mode_t mode;
	int fd, semid;
	char filename[PATH_MAX];
	unsigned int value;
	va_list ap;
	va_start(ap, oflag);
	mode = va_arg(ap, mode_t);
	value = va_arg(ap, unsigned int);
	va_end(ap);
	printf("%d %u\n", mode, value);
		
	umask(0);
	sprintf(filename, "/dev/shm/sem.%s", name);
	fd = open(filename, O_CREAT, mode);
	close(fd);
	

	// create System V semaphore
	semid = semget(ftok(filename, 1), 1, IPC_CREAT | mode);
	if (semid == -1) {
		return SEM_FAILED;
	}

	if (init == 0) {
		memset(semidv, 0, sizeof(semidv));
		init = 1;
	}
	// initialize semaphore (this isnt atomic as it should be)
	union semun arg;
	arg.val = value;
	if (semctl(semid, 0, SETVAL, arg) == -1)
		return SEM_FAILED;

	semidv[semid].semid = semid;
	semidv[semid].cnt += 1;
	strcpy(semidv[semid].filename, name);
	return &semidv[semid].semid;
}
int sem_getvalue(sem_t *sem, int *sval) {
	*sval = semctl(*sem, 0, GETVAL);

}
int sem_post(sem_t *sem) {
	struct sembuf sop;
	sop.sem_op = 1;
	sop.sem_flg = 0;
	sop.sem_num = 0;

	return semop(*sem, &sop, 1);
}
int sem_wait(sem_t *sem) {
	struct sembuf sop;
	sop.sem_op = -1;
	sop.sem_flg = 0;
	sop.sem_num = 0;

	return semop(*sem, &sop, 1);
}
int sem_destroy(sem_t *sem) {
	semidv[*sem].cnt--;
	if (semidv[*sem].unlink == 1 && semidv[*sem].cnt <= 0) {
		// remove from semid vector and filesystem
		char filename[PATH_MAX];
		sprintf(filename, "/dev/shm/sem.%s", semidv[*sem].filename);
		if (unlink(filename) == -1)
			return -1;
		memset(&semidv[*sem], 0, sizeof(Semobj));	
	}
}
int sem_unlink(sem_t *sem) {
	semidv[*sem].unlink = 1;
}

/**
 * Driver program to test this System V implmentation of POSIX semaphores.
 */
int main() {
	int val;
	sem_t *sem = sem_open("hi", O_RDWR, S_IRUSR | S_IWUSR, 1);
	if (sem == SEM_FAILED) {
		perror("sem_open");
		return -1;
	}
	sem_getvalue(sem, &val);
	if (val == -1 && errno != 0) {
		perror("sem_getvalue");	
		return -1;
	}
	printf("semaphore is: %d\n", val);

	sem_unlink(sem);
	if (sem_destroy(sem) == -1) {
		perror("sem_destroy");
		return -1;
	}

	printf("Removed semaphore\n");
	
	// this should produce an error
	sem_getvalue(sem, &val);
	if (val == -1 && errno != 0) {
		perror("sem_getvalue");	
		return -1;
	}
	printf("semaphore is: %d\n", val);
}
