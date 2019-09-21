/**
 * Simple messaging client using POSIX msg queues, see chat_server.c for more details.
 */
#include "chat.h"
#include <pwd.h>

/**
 * Child process that listens on this users POSIX message queue and 
 * prints the messages to stdin
 */
static int receive(char* user) {
	Message msg;
	char mq_name[NAME_MAX] = "/";
	struct mq_attr attr;
	
	strcat(mq_name, user); attr.mq_msgsize = sizeof(Message);
	attr.mq_maxmsg = 10;

	umask(0);
	mqd_t mqd = mq_open(mq_name, O_RDONLY | O_CREAT, perms, &attr);
	if (mqd == -1) {
		perror("client: open");
		_exit(EXIT_FAILURE);
	}
	for (;;) {
		if (mq_receive(mqd, (char*)&msg, attr.mq_msgsize, NULL) == -1)
			perror("client: receive");	
		else
			printf("%s", msg.data);

	}
}
int main(int argc, char *argv[]) {
	struct mq_attr attr;
	struct passwd* pwd;
	Message msg;
	mqd_t mqd;

	if (argc < 2) {
		printf("please enter a person to talk to\n");
		exit(EXIT_FAILURE);
	}

	pwd = getpwuid(getuid());	
	if (pwd == NULL) {
		perror("client: getpwuid");
		exit(EXIT_FAILURE);
	}

	strcpy(msg.to, argv[1]);
	strcpy(msg.from, pwd->pw_name);

	mqd = mq_open(SERVER, O_WRONLY); 
	if (mqd == -1) {
		perror("client open");
		exit(EXIT_FAILURE);
	}

	if (mq_getattr(mqd, &attr) == -1) {
		perror("client getattr");
		exit(EXIT_FAILURE);
	}

	// spawn a child to receive messages
	if (fork() == 0) {
		receive(pwd->pw_name);
		_exit(EXIT_SUCCESS);	
	}

	// loop every 
	while (fgets(msg.data, BUFFER_SIZE, stdin)) {
		// dont send empty strings
		if (msg.data[0] == '\n') 
			continue;	
			
		if (mq_send(mqd, (char*)&msg, attr.mq_msgsize, 0) == -1) {
			perror("client: send");
		}
	}
	
}
