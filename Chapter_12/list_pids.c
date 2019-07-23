#include <stdio.h>
#include <stdlib.h>
#include <pwd.h>
#include <limits.h>
#include <dirent.h>
#include <errno.h>
#include <string.h>

uid_t id_to_name(const char* name) {
	struct passwd* pwd = getpwnam(name);
	
	return pwd != NULL ? pwd->pw_uid : -1;
}

/**
 * check if user id matches the real id field in the PID status file
 * Returns 1 on match else 0 
 */
int is_user(uid_t id, FILE* fp, char* name) {
	uid_t real_id = -1;
	char buf[256];
	
	// look for user id field in status file
	// cannot assume that line numbers are portable
	while (fgets(buf, 256, fp) != NULL) {
		if (strncmp(buf, "Name:", 5) == 0) {
			sscanf(buf, "Name: %s", name);
		}
		// Uid field has the following format:
		// Uid: [real id] [effective id] [saved set id] [file system id]
		if (strncmp(buf, "Uid:", 4) == 0) {
			sscanf(buf, "%*s %d ", &real_id);
			break;
		}
	}
	if (real_id == id) return 1;
	return 0;
}

int main (int argc, char* argv[]) {
	uid_t id; 
	char path[PATH_MAX] = "/proc/";	
	char name[NAME_MAX];
	
	if (argc < 2) {
		printf("Missing user argument\n");
		return -1;
	}

	id = id_to_name(argv[1]);
	if (id == -1) {
		printf("User not found\n");
		return -1;
	}

	DIR* dirp = opendir(path);
	struct dirent* direntp;

	// walk through each directory entry
	while( (direntp = readdir(dirp)) != NULL) {
		// skip all directories that are not a PID,
		if (strtol(direntp->d_name, NULL, 10) == 0) continue;
		
		// overwrite the directory name after the /proc/ (6 bytes later)	
		strcpy(path + 6, direntp->d_name);
		strcat(path, "/status");

		FILE* fp = fopen(path, "r"); 
		if (fp != NULL && is_user(id, fp, name)) {
			printf("%s %s\n", direntp->d_name, name);
		} 
		fclose(fp);

	}
	closedir(dirp);

}
