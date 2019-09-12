#include <stdio.h>
#include <sys/ipc.h>
#include <sys/types.h>
#include <sys/msg.h>
#include <sys/stat.h>

#define SEQ_MULTILPLIER 32768 // defined in include/linux/ipc.h

/**
 * Verfires algorithim used in Systems V kernel data structure
 */
int main() {
	int id, calc_id, index;
	struct msqid_ds buf;
	
	// get a key for the message
	key_t key = ftok("test", 'x'); // path must exist
	if (key == -1) {
		perror("key");
		return -1;
	}

	id = msgget(key, IPC_CREAT | S_IRUSR);
	if (id == -1) {
		perror("msgget");
		return -1;
	}

	if (msgctl(id, IPC_STAT, &buf) == -1) {
		perror("msgctl");
		return -1;
	}

	struct ipc_perm perm = buf.msg_perm;

		
	// calculate the indentifier returned by msgget
	index = id%SEQ_MULTILPLIER;
	calc_id = index + perm.__seq*SEQ_MULTILPLIER; 
	// the calculated id should be equal to id
	printf("real id: %ld calculated id: %ld\n", id, calc_id);


}
