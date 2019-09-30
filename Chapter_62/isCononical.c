/******************************************************************************
 * Simple program to that displays weather or not the terminal is in canonical
 * mode. If the terminal is not, the TIME and MIN feilds are displayed.
 * The program uses stdin as the terminals file stream to display.
 ******************************************************************************/ 
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <termios.h>

int main() {
	struct termios tp;
	if (tcgetattr(STDIN_FILENO, &tp) == -1) {
		perror("tcgetattr");
		return -1;
	}
	if (tp.c_lflag & ICANON) {
		printf("Canonical\n");
	} else {
		printf("Noncanonical TIME: %d tenths of a second MIN: %d\n",
			       	tp.c_cc[VTIME], tp.c_cc[VMIN]);
	}

}
