#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>

static char buf[256];
/**
 * Implmentation of the obsolete getpass function, see getpass(3)
 * for more details.
 */
char *getpass(const char *prompt) {
	struct termios tp, stp;
	int fd = open("/dev/tty", O_RDWR);	
	if (fd == -1)
		return NULL;

	if (write(fd, prompt, strlen(prompt)) != strlen(prompt)) 
		return NULL;
	
	if (tcgetattr(fd, &tp) == -1)
		return NULL;
	stp = tp; // save the terminal state

	// turn off echoing
	tp.c_lflag &= ~ECHO;
	// change occurs after all is wrote to fd
	if (tcsetattr(fd, TCSAFLUSH, &tp) == -1)
		return NULL;

	// read one line
	if (read(fd, buf, sizeof(buf)) == -1)
		return NULL;

	// resotre terminal state
	if (tcsetattr(fd, TCSAFLUSH, &stp) == -1)
		return NULL;

	return buf;
}


/**
 * Driver program to test getpass
 */
int main() {
	printf("%s", getpass("Password: "));	
}
