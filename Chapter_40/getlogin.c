#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <utmpx.h>
#include <paths.h>
#include <string.h>
#include <libgen.h>

#define _UT_NAMESIZE 32
static char buf[_UT_NAMESIZE];

/**
 * Implmenetation of getlogin(3), see man page for details
 * Note: must use appropriate terminal emulator like xterm or
 * the virtual consoles
 */
char* getlogin() {
	struct utmpx tmp;
	struct utmpx* res = NULL;
	char* tty;

	tty = ttyname(STDIN_FILENO);
	strcpy(tmp.ut_line, tty+5); // remove /dev/ from tty and copy
	tmp.ut_type = LOGIN_PROCESS;	

	// rewind utmp file
	setutxent();

	// clear cache
	if (res != NULL)
		memset(res, 0, sizeof(struct utmpx));	


	res = getutxline(&tmp);
	if (res == NULL)
		return NULL;
	
	strcpy(buf, res->ut_user);
	endutxent();
	return buf;
}


/**
 * Driver program for testing getlogin
 */
int main() {
	char* str = getlogin();
	if (str == NULL) {
		perror("getlogin");
		return -1;
	}
	printf("%s\n", str);
}
