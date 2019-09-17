#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <wait.h>

#define PERMS (S_IWUSR | S_IRUSR)

typedef struct {
	char key[256];
	int val;
} Pair;

#define DIR_SIZE (1000*sizeof(Pair))

static Pair* dir;

union semun {
	int val;	
	struct semid_ds *buf;
	unsigned short* array;
};
#define SHM_KEY 0xaabbccdd
#define SEM_KEY 0xaabbccdd
#define READ_SEM 0
#define WRITE_SEM 1

static int shmid, semid;

/**
 * Create a semahpore protected shared memory segment
 * Provides add,delete and get functionality that is
 * multiprocess safe.
 *
 * Uses a read and write semahpore that each counts the total number of 
 * readers and writers to allow multiple readers concurrently.
 * Readers can run when there are fdxo writers, writers must wait for all other 
 * readers and writers to complete.
 */
int initDir() {
	struct sembuf sb;
	union semun arg;
	shmid = shmget(SHM_KEY, DIR_SIZE, IPC_CREAT | PERMS);
	if (shmid == -1)
		return -1;
	
	dir = shmat(shmid, NULL, 0);
	if (*(int*)dir== -1)
		return -1;

	// remove shared memory once finished
	if (shmctl(shmid, IPC_RMID, 0) == -1)
		return -1;
	
	// get the semphore if it exists, else create it
	semid = semget(SEM_KEY, 2, PERMS);
	if (semid == -1 && errno == ENOENT) { 
		memset(dir, 0, DIR_SIZE);
		// sempahore does not exist
		semid = semget(SEM_KEY, 2, PERMS | IPC_CREAT | IPC_EXCL);

		// initialize semaphore 
		arg.val = 0;
		if (semctl(semid, READ_SEM, SETVAL, arg) == -1)
			return -1;
		if (semctl(semid, WRITE_SEM, SETVAL, arg) == -1)
			return -1;
	}
       	if (semid == -1)
		return -1;
		
	return 0;
}

/**
 * Waits blocks until there are no more readers and increments the amount of 
 * writers. These two steps have to be performed by the kernel to avoid race
 * conditions.
 */
static int getWritePerm() {
	struct sembuf sb_arr[2];
	
	// wait for readers to finish	
	sb_arr[READ_SEM].sem_num = READ_SEM;
	sb_arr[READ_SEM].sem_op = 0;
	sb_arr[READ_SEM].sem_flg = 0;
	
	// increment the numbers of writers
	sb_arr[WRITE_SEM].sem_num = WRITE_SEM;
	sb_arr[WRITE_SEM].sem_op = 1;
	sb_arr[WRITE_SEM].sem_flg = 0;
	if (semop(semid, sb_arr, 2) == -1)
		return -1;
	return 0;
}
static int remWritePerm() {
	struct sembuf sb;
	sb.sem_num = WRITE_SEM;
	sb.sem_op = -1;
	
   	if (semop(semid, &sb, 1) == -1)
		return -1;

	return 0;
}

/**
 * Increments the number of readers and blocks 
 * until there are no more readers.
 * These two steps have to be peformed synchronously by the kernel 
 * to avoid multipler readers causing a race condition.
 *
 * An alternative would be to have each process use the reader semaphore as a
 * mutex that guards while the process increments a global count of readers. If this is
 * the first process, the shared memory is locked as well. If a writer is already
 * running the reader will block, else the writer will be blocked.
 * Once the are readers are finished, they lock the reader sempahore to
 * decrement the reader counter. The last reader will also unlock the shared memory sempahore.
 * The writer will then be able to run. 
 */
static int getReadPerm() {
	struct sembuf sb_arr[2];
	// increment number of readers
	sb_arr[READ_SEM].sem_num = READ_SEM;
	sb_arr[READ_SEM].sem_op = 1;
	sb_arr[READ_SEM].sem_flg = 0;

	// wait for 0 writers to read
	sb_arr[WRITE_SEM].sem_num = WRITE_SEM;
	sb_arr[WRITE_SEM].sem_op = 0;
	sb_arr[WRITE_SEM].sem_flg = 0;
	if (semop(semid, sb_arr, 2) == -1)
		return -1;
	return 0;
}
static int remReadPerm() {
	struct sembuf sb;
	sb.sem_num = READ_SEM;
	sb.sem_op = -1;
	
	// decrement this reader
   	if (semop(semid, &sb, 1) == -1)
		return -1;

	return 0;
}

int add(char* key, int val) {
	if (getWritePerm() == -1)
		return -1;
	// linear search for key
	size_t i;
	for(i = 0; i < DIR_SIZE/sizeof(Pair); i++)
		if (dir[i].key[0] == '\0' || strcmp(dir[i].key, key) == 0) {
			strcpy(dir[i].key, key);
			dir[i].val = val;
			break;
		}
			
	if (i == DIR_SIZE/sizeof(Pair)) {
		errno = E2BIG;
		return -1;
	}

	if (remWritePerm() == -1)
		return -1;

	return 0;
}

int del(char* key) {
	int result = -1;
	errno = ENONET;
	if (getWritePerm() == -1)
		return -1;
	// linear search for key
	size_t i;
	for(i = 0; i < DIR_SIZE/sizeof(Pair); i++)
		if (strcmp(dir[i].key, key) == 0) {
			memset(&dir[i], 0, sizeof(Pair));
			errno = 0;
			result = 0;
			break;
		}

	if (remWritePerm() == -1)
		return -1;

	return result;
}

int get(char* key) {
	if (getReadPerm() == -1)
		return -1;
	int result = -1;
	errno = ENOENT;	

	// linear search for key
	size_t i;
	for(i = 0; i < DIR_SIZE/sizeof(Pair); i++)
		if (strcmp(dir[i].key, key) == 0) {
			result = dir[i].val;
			errno = 0;
		}

	printf("[%d] read: %s\n", getpid(), dir[i].key);
	if (remReadPerm() == -1)
		return -1;
	return result;
}

int closeDir() {

}

/**
 * Driver program to test directory API
 */
int main() {
	int res;
	// call initDir before hand
	if (initDir() == -1) {
		perror("initDir");
		return -1;
	}
	/* create another process to concurrently change directory*/
	switch(fork()) {
		case -1:
			return -1;	
		case 0:
			if (add("test", 1) == -1)  {
				perror("add");
				return -1;
			}
			sleep(1);
			if (del("test two") == -1){
				perror("del");
				return -1;
			}
			sleep(2);
			_exit(EXIT_SUCCESS);
		default:
			break;


	}

	sleep(1);
	if (add("test two", 0) == -1)  {
		perror("add");
		return -1;
	}
	if ((res = get("test")) == -1) {
		perror("get");
		return -1;
	}
	printf("%d\n", res);

	wait(NULL);

	return 0;
}
