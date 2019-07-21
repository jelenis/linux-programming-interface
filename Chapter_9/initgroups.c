#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <limits.h>
#include <grp.h>

/**
 * Implementation of initgroups systemcall, needs root access to change preliminary groups
 * using setgroups(2) system call
 */


int initgroups(const char* user, gid_t group);

int initgroups(const char* user, gid_t group){
	gid_t group_ids[NGROUPS_MAX + 1];

	errno = 0;	
	if (getuid() != 0) {
		errno = EPERM;

		printf("%s\n", strerror(errno));
		return -1;
	}

	FILE* fp = fopen("/etc/group", "r");
	if (fp == NULL) {
		strerror(errno);
		return -1;
	}
	
	char buf[256];
	char name[256];
	char pass[256];
	char members[512];
	long id;
	size_t i = 0; 
	// scan group file line by line and parse search for the
	// user name in the group member feild
	while (fgets(buf, 256, fp) != NULL) {
		sscanf(buf, "%[^:]:%*[^:]:%ld:%s", name, &id, members);
		char* sv;
		char* tok = strtok_r(members, ",", &sv);

		while(tok != NULL) {
			if (strcmp(user,tok) == 0) {
				group_ids[i] = (gid_t)id;
				i ++;
			}
			tok = strtok_r(NULL, ",", &sv);
		}
	}	
	fclose(fp);

	if (group >= 0) {
		group_ids[i] = (gid_t)group;
		i ++;
	}
	if(setgroups(i, group_ids) == -1) {
		printf("%s\n", strerror(errno));
		return -1;
	}
	return 0;
}

/** 
 * driver program to test initgroups
 * must run as root to change process preliminary groups
 */
int main() {
	int len = 0;
	gid_t l[NGROUPS_MAX + 1]; // list to hold all the process groups

	// my user name
	initgroups("jay", 0);
	if ((len = getgroups(100, l)) == -1) {
		printf("%s\n", strerror(errno));
	}
	for (int i=0; i< len; i++) printf("%d\n", l[i]);
}
