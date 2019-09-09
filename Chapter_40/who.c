#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <utmpx.h>
#include <paths.h>
#include <pwd.h>
#include <string.h>
#include <time.h>

/**
 * Implementation of who(1) bash command.
 * Only supports showing who is logged in.
 */
int main(int argc, char* argv[]) {

	struct passwd* pwd;
	struct utmpx* res = NULL;
	
	// perform linear search for each user
	while ((pwd = getpwent()) != NULL) {
		setutxent(); // rewind utmp to begining and repeat
		while ((res = getutxent()) != NULL) {
			if (res->ut_type == USER_PROCESS 
					&& strcmp(res->ut_user, pwd->pw_name) == 0) {
				time_t t = res->ut_tv.tv_sec;
				// new line is appendined at end of ctime
				printf("%s %s %s", res->ut_user, res->ut_line, ctime(&t));
			}
		}
	}


	endutxent();
}
