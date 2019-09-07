#include <stdio.h>
#define _XOPEN_SOURCE
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <shadow.h>
#include <pwd.h>
#include <errno.h>

/**
 * douser a simple implmentation of sudo(1).
 * Use -u to specify a user to run a program as.
 * Must be compiled with -lcrypt and have the set-user-id bit set for a root owner
 */
int main(int argc, char* argv[]) {
	char user[256] = "root"; // assume 256 is large enough
	char* pass;
	char* encrypt;
	struct passwd* pwd;
	struct spwd* spwd;
	int opt;

	while ((opt = getopt(argc, argv, "u:")) != -1) {
		if (opt == 'u') {
			strcpy(user, optarg);
			break;
		}		
	}
	if (argv[optind] == NULL) {
		printf("No program\n");
		exit(EXIT_FAILURE);
	}	

	/* get the encrypted pwd record from pwd file and the associated password 
	from the shadow file */
	pwd = getpwnam(user);
	if (pwd == NULL) {
		printf("User %s does not exist\n", user);
		exit(EXIT_FAILURE);
	}

	spwd = getspnam(user);
	if (spwd == NULL && errno == EACCES) {
		perror("getpwnam");
		exit(EXIT_FAILURE);
	} else if (spwd != NULL) {
		// use shadow password, if it exists
		pwd->pw_passwd = spwd->sp_pwdp;
	}
	
	// prompt user for password
	pass = getpass("Password: ");

	/* encrypt password, the first two characters of the
	encrypted password is the salt */
	encrypt = crypt(pass, pwd->pw_passwd);

	// erase every character of password (do as fast as possible)
	for (char* p = pass; *p != '\0'; p++)
		*p = '\0';	
	
	if (encrypt == NULL) {
		perror("crypt");
		exit(EXIT_FAILURE);
	}

	// compare the two encrypted passwords 
	if (strcmp(encrypt, pwd->pw_passwd) != 0) {
		printf("Incorrect password\n");
		exit(EXIT_FAILURE);
	}


	// set process and group ids to this user to drop privleges
	if (setregid(pwd->pw_gid, pwd->pw_gid) == -1) {
		perror("setregid");
		exit(EXIT_FAILURE);
	}
	if (setreuid(pwd->pw_uid, pwd->pw_uid) == -1) {
		perror("setreuid");
		exit(EXIT_FAILURE);
	}	
	
	// exec program 
	if (execvp(argv[optind], &argv[optind]) == -1) {
		perror("execvp");
		exit(EXIT_FAILURE);
	}

}
