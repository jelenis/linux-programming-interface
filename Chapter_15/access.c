#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<fcntl.h>
#include<errno.h>
#include<sys/stat.h>
#include<sys/types.h>
#include<pwd.h>
#include<errno.h>
#include<grp.h>
#include<limits.h>

/**
 * Perform linear search on grouplist to see if the file is part of
 * one of the supplimentery groups
 *
 * returns 1 on found else 0
 */
int in_groups(gid_t file_grp, const gid_t* groups, int ngroups) {
	for (int i = 0; i < ngroups; i++) {
		if (file_grp == groups[i]) return 1;
	}	
	return 0;	
}
/**
 * Check if the effective user has permission to open the file
 * the systemcall equalvilent is euidaccess(3)
 *
 * Compares the effective uid and the associated groups with the
 * file permission bits and determines if the file can be accessed
 * given by the flags in mode
 * 
 * Note: uses same mode flags as seen in: access(3)
 * 	R_OK, W_OK, X_OK, F_OK
 */
int has_access(const char* pathname, int mode) {
	struct passwd* pw;
	struct stat statbuf;
	int access = 0;
	
	//get effective id
	uid_t euid = geteuid();	
	getpwuid(euid);

	if (pathname == NULL) {
		errno = ENOENT; 
		perror("Empty path");
		return -1;
	}

	if (pw == NULL) {
		perror("Retrieving user information");
		return -1;
	}

	if (stat(pathname, &statbuf) == -1){
		if (mode & F_OK && errno == ENOENT) return -1;
		perror("Opening file");	
		return -1;
	}
	int ngroups = sysconf(_SC_NGROUPS_MAX) + 1;
	gid_t* groups = malloc(ngroups*sizeof(gid_t));
	if (getgrouplist(pw->pw_name, pw->pw_gid, groups, &ngroups) == -1){
		perror("Loading supplementary groups");
		return -1;
	}
	
	if (statbuf.st_uid == euid) {
		// file owner match  
		if (statbuf.st_mode & S_IRUSR) access |= R_OK;
		if (statbuf.st_mode & S_IWUSR) access |= W_OK;
		if (statbuf.st_mode & S_IXUSR) access |= X_OK;
	} else if (statbuf.st_gid == pw->pw_gid 
			|| in_groups(statbuf.st_gid, groups, ngroups)) {
		// group match
		if (statbuf.st_mode & S_IRGRP) access |= R_OK;
		if (statbuf.st_mode & S_IWGRP) access |= W_OK;
		if (statbuf.st_mode & S_IXGRP) access |= X_OK;
	} else {
		// other user
		if (statbuf.st_mode & S_IROTH) access |= R_OK;
		if (statbuf.st_mode & S_IWOTH) access |= W_OK;
		if (statbuf.st_mode & S_IXOTH) access |= X_OK;
	}	
	
	if (mode & R_OK && (access & R_OK) != R_OK) return -1;
	if (mode & W_OK && (access & W_OK) != W_OK) return -1;
	if (mode & X_OK && (access & X_OK) != X_OK) return -1;

	return 1;
}

/**
 * Driver for has_access function
 */
int main (int argc, char* argv[]) {
	printf("%d\n", has_access(argv[1], R_OK));
	
	return 0;
}
