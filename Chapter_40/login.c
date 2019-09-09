#define _GNU_SOURCE
#include <stdio.h>
#include <paths.h>
#include <utmpx.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>

/**
 * Implmentation of login(3), see man page for details
 */
void login(const struct utmpx* ut) {
	setutxent();
	if (pututxline(ut) == NULL)
		return;
	updwtmpx(_PATH_WTMP, ut);
	endutxent();
}

/**
 * Implementation of logout(3), see man page for details
 */
int logout(const char* ut_line) {
	struct utmpx* res = NULL;
	struct utmpx ut;
	strcpy(ut.ut_line, ut_line);
	ut.ut_type = USER_PROCESS;

	setutxent(); // rewind
	res = getutxline(&ut); // seek to record
	if (res == NULL)
		return 0; // 0 on error

	// zero out record and overwite
	memset(&ut, 0, sizeof(struct utmpx));
	if (pututxline(&ut) == NULL)
		return 0; // 0 on error

	endutxent();
}

/**
 * Implementation of logwtmp(3), see man page for details
 */
void logwtmp(const char* line, const char* name, const char* host) {
	struct utmpx ut;
	memset(&ut, 0, sizeof(struct utmpx));
	strcpy(ut.ut_line, line);
	strcpy(ut.ut_user, name);
	strcpy(ut.ut_host, host);
	updwtmpx(_PATH_WTMP, &ut); // append to end of file

}


/**
 * Driver function to test out login, logout & logwtmp
 * Note: need to run under proper terminal emulators like xterm or a virtual
 * console with root privilege
 */
int main(int argc, char* argv[]) {
	struct utmpx ut;
	char* devName = ttyname(STDIN_FILENO);

	// zero out contents of ut
	memset(&ut, 0, sizeof(struct utmpx));
	strcpy(ut.ut_line, devName + 5);
	strcpy(ut.ut_id, devName + 8);
	strcpy(ut.ut_user, "user");
	ut.ut_type = USER_PROCESS;
	ut.ut_pid = getpid();
	time((time_t*) &ut.ut_tv.tv_sec);

	login(&ut);

	//if (logout(devName + 5) == 0)
		//perror("logout");
	logwtmp(devName + 5, devName + 8, "jay");
	return 0;
}
