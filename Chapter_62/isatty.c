#include <termios.h>

/**
 * isatty implmented with a call to tcgetattr(3)
 */
int isatty(int fd) {
	struct termios t;
	// 1 on success, 0 on failure
	return tcgetattr(fd, &t) + 1; 
}
