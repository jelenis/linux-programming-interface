#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/acl.h>
#include <sys/types.h>
#include <acl/libacl.h>
#include <errno.h>
#include <grp.h>
#include "../lib/tlpi_hdr.h"
#include "ugid_functions.h"

/**
 * Prints out permissions in the standard rwx form
 * as seen with: ls -l file
 *
 * Returns -1 on failure, 0 on success
 */
int print_perms(acl_entry_t entry) {
	acl_permset_t permset;	
	int perm;

	if (acl_get_permset(entry, &permset) == -1)
		return -1;

	perm = acl_get_perm(permset, ACL_READ);
	if (perm == -1) return -1;
	printf("%c", (perm == 1) ? 'r' : '-');

	perm = acl_get_perm(permset, ACL_WRITE);
	if (perm == -1) return -1;
	printf("%c", (perm == 1) ? 'w' : '-');
	
	perm = acl_get_perm(permset, ACL_EXECUTE);
	if (perm == -1) return -1;
	printf("%c\n", (perm == 1) ? 'x' : '-');

	return 0;
}

/**
 * Prints out the masked version of the group permisions
 *
 * Returns -1 on failure, 0 on success
 */
int print_masked(acl_entry_t entry, acl_entry_t mask_entry) {
	acl_permset_t permset;	
	acl_permset_t mask_permset;
	int perm;
	int mask;

	if (acl_get_permset(entry, &permset) == -1)
		return -1;
	if (acl_get_permset(mask_entry, &mask_permset) == -1)
		return -1;

	perm = acl_get_perm(permset, ACL_READ);
	if (perm == -1) return -1;
	mask = acl_get_perm(mask_permset, ACL_READ);
	if (mask == -1) return -1;
	printf("%c", (perm & mask == 1) ? 'r' : '-');

	perm = acl_get_perm(permset, ACL_WRITE);
	if (perm == -1) return -1;
	mask = acl_get_perm(mask_permset, ACL_WRITE);
	if (mask == -1) return -1;
	printf("%c", (perm & mask == 1) ? 'w' : '-');

	perm = acl_get_perm(permset, ACL_EXECUTE);
	if (perm == -1) return -1;
	mask = acl_get_perm(mask_permset, ACL_EXECUTE);
	if (mask == -1) return -1;
	printf("%c\n", (perm & mask == 1) ? 'x' : '-');

	return 0;
}

/**
 * Does a simple linear search to check if group is in groups
 * Returns 1 if located else 0
 */
int in_groups(gid_t group, gid_t groups[], int ngroups) {
	for (int i = 0; i < ngroups; i++) 
		if (groups[i] == group) return 1;	
	return 0;
}

/**
 * Prints out the ACL entries associated with specific users and groups,
 * also diplays the masked output for groups.
 *
 * Creates a file name "test" in the local directory and uses this file to
 * analyze the ACL.
 */
int main(int argc, char* argv[]){
	uid_t uid;
	gid_t gid;
	int user_mode = -1;
	int entry_id = ACL_FIRST_ENTRY;
	int check_mask = 0;
	acl_t acl;
	acl_entry_t entry;
	acl_entry_t group_entry;
	acl_tag_t tag;
	acl_type_t type = ACL_TYPE_ACCESS;
	char* path = "test";
	char user[10]; // 10 should be sufficient usernames
	char group[10];	
	
	// parse command line options, u for user, g for group
	int opt;
	while ((opt = getopt(argc, argv, "u:g:")) != -1) {
		switch(opt) {
			case 'u':
				strcpy(user, optarg);
				user_mode = 1;
				break;
			case 'g':
				strcpy(group, optarg);
				user_mode = 0;
				break;
		}
	}
	
	// get the groups that this belongs to 
	int ngroups = 100;
	gid_t groups[ngroups];
	if (user_mode == 1) {
		uid = userIdFromName(user);
		if (getgrouplist(userNameFromId(uid), groupIdFromName(user),
				       	groups, &ngroups) == -1) {

			perror("getgrouplist");
			return -1;
		}
	} else if (user_mode == 0) {
		gid = groupIdFromName(group);
	} else {
		// if user mode is not 0 or 1, then there were missing paramters
		printf("Use -u or -g to specify a user or group.\n");
		return -1;
	}

	// create a if it doesnt already exist 
	FILE* fp = fopen(path, "r");
	if (fp == NULL) {
		perror("Creating test file");
		return -1;
	}
	fclose(fp);

	acl = acl_get_file(path, type);
	if (acl == NULL) {
		perror("acl_get_file");
		return -1;
	}

	// calculate the mask to guarantee it exists
	if (acl_calc_mask(&acl) == -1) {
		perror("acl_calc_mask");
		return -1;
	}

	// loop through acl list entries
	while (acl_get_entry(acl, entry_id, &entry) == 1) {
		// use the entry to get the type
		if (acl_get_tag_type(entry, &tag) == -1) {
			perror("acl_get_tag_type");
			return -1;
		}

		/* Write user and group entries from the ACL list. If 
		   the user happens to be found by a group they belong to, 
		   print the masked effective access when the mask is applied */
		if (tag == ACL_USER && user_mode == 1) {
			printf("ACL_USER ");
			uid_t* qualifier = acl_get_qualifier(entry);

			if (qualifier == NULL) {
				perror("acl_get_qualifier");
				return -1;
			}
			if (*qualifier == uid) {
				if (print_perms(entry) == -1) {
					perror("print_perms");
					return -1;
				}	
			}		
			if (acl_free(qualifier) == -1) {
				perror("acl_free");
				return -1;
			}

		} else if (tag == ACL_GROUP) {
			printf("ACL_GROUP ");
			gid_t* qualifier = acl_get_qualifier(entry);

			if (qualifier == NULL) {
				perror("acl_get_qualifier");
				return -1;
			}
			if (*qualifier == gid || in_groups(gid,groups,ngroups)) {
				if (print_perms(entry) == -1) {
					perror("print_perms");
					return -1;
				}
				group_entry = entry;
				check_mask = 1;		
			}

			if (acl_free(qualifier) == -1) {
				perror("acl_free");
				return -1;
			}

		} else if (check_mask && tag == ACL_MASK) {
			/* print ANDed result of group to apply the
			   group mask */
			printf("Group Mask Result ");
			print_masked(group_entry, entry);
		}

		entry_id = ACL_NEXT_ENTRY;
	}	
	if (acl_free(acl) == -1) {
		perror("acl_free");
		return -1;
	}
}

