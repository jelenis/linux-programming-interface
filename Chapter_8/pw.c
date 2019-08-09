#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <string.h>
#include <errno.h>
//#include <pwd.h>
// pwnam statically allocates struct (this is not thread safe)
struct passwd {
	char* pw_name;
	char* pw_passwd;
	uid_t pw_uid;
	gid_t pw_gid;
	char* pw_gecos;
	char* pw_dir;
	char* pw_shell;
};

// statically allocated strings
char pw_name[256];
char pw_passwd[256];
char pw_gecos[256];
char pw_dir[256];
char pw_shell[256];

struct passwd pwd; 
struct passwd* getpwnam(const char* name);

// parse feilds  
void read_user(char* token) {
	int member = 0;
	strcpy(pw_name, token);
	// TODO: use sscanf instaed
	while (token != NULL) {
		if(token[-1] != ':')// check for empty field
		switch (member) {
			case 1:
				strcpy(pw_passwd, token);
				break;
			case 2:
				pwd.pw_uid = strtol(token, NULL, 10);
				break;
			case 3:
				pwd.pw_gid = strtol(token, NULL, 10);
				break;
			case 4:
				strcpy(pw_gecos, token);
				break;
			case 5:
				strcpy(pw_dir, token);
				break;
			case 6:
				strcpy(pw_shell, token);
				break;
		}
		token = strtok(NULL, ":");
		member += 1;
	}

}

/**
 * Custom implmentation of pwnam, with statically allocated data
 * Returns NULL on error
 */
struct passwd *getpwnam(const char* name) {
	FILE * fp = fopen("/etc/passwd", "r");
	struct passwd* ptr = NULL;
	if (fp == NULL) {
		printf("%s\n", strerror(errno));
		return NULL;
	}
	char buf[512];

	// search for user name in /etc/passwd
	// once found parse the fields and load them into a struct
	while (fgets(buf, 512, fp) != NULL) {
		char* token = strtok(buf, ":");

		// found the user
		if (strcmp(token, name) == 0) {
			read_user(token);
			pwd.pw_name = pw_name;
			pwd.pw_passwd = pw_passwd;
			pwd.pw_gecos = pw_gecos;
			pwd.pw_dir = pw_dir;
			pwd.pw_shell = pw_shell;	
			
			// set the return value
			ptr = &pwd;
			break;
		}
			
	}

	
	fclose(fp);
	return ptr;	
}

// driver program to test implmentation of getpwnam
int main() {
	struct passwd* my_pwd = getpwnam("root");
	printf("PWD\n------------\n\t%s\n", my_pwd->pw_name);
	printf("\t%s\n", my_pwd->pw_gecos);
	return 0;
}
