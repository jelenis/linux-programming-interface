/**
 * simple chat client like talk(1)
 */
#include "chat.h"
#include <string.h>
static int myid, serverid;

/**
 * Disconnect from chat server and remove system queue
 */
static void clean_up(int signum) {
	Message msg;	
	msg.mtype = M_DISCON;
	msg.from = getuid();

	// send disconnect message to the server
	if (msgsnd(serverid, &msg, sizeof(uid_t), 0) == -1) {
		perror("msgsnd");
		exit(EXIT_FAILURE);
	}

	// remove message queue
	if (msgctl(myid, IPC_RMID, NULL) == -1) {
		perror("msgctl");
		exit(EXIT_FAILURE);
	}
	signal(signum, SIG_DFL);
	if (raise(signum) != 0)
		exit(EXIT_FAILURE);
}

/**
 * Runs in a child process to print incoming text to the terminal
 */
static void receive() {
	Message resp;
	for (;;) {
		if (msgrcv(myid, &resp, MSG_SIZE, M_DATA, 0) == -1) {
			perror("msgrcv");
			exit(EXIT_FAILURE);
		} else {
			printf("%s", resp.data);
		}
	}
}

/**
 * Requests a sequence of numbers from the server
 * using the length specified in argv[2]
 */
int main (int argc, char* argv[]) {
	Message req, msg;
	uid_t uid, ruid;
	struct passwd* pwd;
	struct sigaction sa;
	char buf[1024];


	if (argc < 2) {
		printf("Enter the username of the receipient\n");
		exit(EXIT_FAILURE);
	}
	
	pwd = getpwnam(argv[1]);
	if (pwd == NULL) {
		fprintf(stderr, "Username [%s] does not exist\n", argv[1]);
		exit(EXIT_FAILURE);	
	}
	uid = getuid();
	ruid = pwd->pw_uid;		
	
	myid = msgget(IPC_PRIVATE, S_IRUSR | S_IWUSR);
	if (myid == -1) {
		perror("msgget myid");
		exit(EXIT_FAILURE);
	}	

	// build request;
	req.mtype = M_CONNECT; 
	req.from = uid;	
	req.msqid = myid;
	
	serverid = msgget(KEY, S_IWUSR | S_IWGRP | S_IWOTH);
	if (serverid == -1) {
		perror("Cannot locate server");
		exit(EXIT_FAILURE);
	}	

	// send connect message to the server
	if (msgsnd(serverid, &req, MSG_SIZE, 0) == -1) {
		perror("msgsnd");
		exit(EXIT_FAILURE);
	}
	
	msg.mtype = M_DATA;
	msg.from = uid;
	msg.to = ruid;
	msg.msqid = myid;

	/* fork a process to handle live output */	
	switch (fork()) {
		case -1:
			perror("fork");	
			exit(EXIT_FAILURE);
		case 0:
			receive();
			exit(EXIT_SUCCESS);
		default:
			break;

	}

	// establish signal handler for exit
	sa.sa_handler = clean_up;
	sa.sa_flags = 0;
	sigaction(SIGINT, &sa, NULL); 
	sigaction(SIGTERM, &sa, NULL); 

	while (fgets(buf, sizeof(buf), stdin) != NULL) {
		size_t len = strlen(buf); // index of '\0'
		if (len > 1){ // do not send empty messages
			strcpy(msg.data, buf);	
			if (msgsnd(serverid, &msg, MSG_SIZE, 0) == -1) {
				perror("failed to send message");
				exit(EXIT_FAILURE);
			}
		}	

		
	}
}
