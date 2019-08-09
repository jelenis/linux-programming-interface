#include <stdio.h>
#include <stdlib.h>
#include <pwd.h>
#include <limits.h>
#include <dirent.h>
#include <errno.h>
#include <string.h>

#define MAX_CHILD 500

typedef struct Node {
	pid_t ppid;
	pid_t pid;
	char name[NAME_MAX];
	int child_count;
	struct Node** children;
} Node;

int map[1024];

/**
 * check if user id matches the real id field in the PID status file
 * Returns 1 on match else 0 
 */
pid_t get_ps(char* path, char* name) {
	pid_t ppid = -1;
	char buf[256];

	FILE* fp = fopen(path, "r");	
	if (fp == NULL) return -1;
	// look for user id field in status file
	// cannot assume that line numbers are portable
	while (fgets(buf, 256, fp) != NULL) {
		sscanf(buf, "Name: %s", name); 
		if (sscanf(buf, "PPid: %d", &ppid) == 1) {
			break;
		}
	}
	fclose(fp);
	return ppid;
}

void dfs(Node* root, int l, int last, int pname) {
	char buf[1024];
	char str[100];
	int current_len;

	if (root == NULL) return;
	memset(buf, 0, 1024);
	
	//printf("%d", pname);
	// check if the first process in this subtree
	if (l == 0) {
		for(int x=0; x < pname ; x++) {
			if (map[x] == 1) strcat(buf, "│");
			else strcat(buf, " ");
			//else strcat(buf, "|");
		}
		if (last){
			strcat(buf, "└─");
		}
		else strcat(buf, "├─");
	} else {
	}
		
	
	sprintf(str, "[%d %s]", root->pid, root->name);
	strcat(buf, str);
	if (l == 0) current_len	= pname + strlen(str) + 3;
	else current_len = pname;
	
	if (root->child_count == 1) {
		strcat(buf, "───");
	}

	else if (root->child_count > 1) {

		map[current_len] = 1;
			//printf("%d on", pname);
       		strcat(buf, "─┬─");
	}



	printf("%s", buf);
	if (root->child_count == 0) {
		printf("\n");
	}

	for (int j = 0; j < root->child_count; j ++) {
		if (j == 0) l = 1;
		else l = 0;	
		
		if (j == root->child_count -1) {
			map[current_len] = 0;
		}
		dfs(root->children[j], l, j == root->child_count - 1, current_len);
	}
}

int main (int argc, char* argv[]) {
	char path[PATH_MAX] = "/proc/";	
	DIR* dirp = opendir(path);
	Node* ps_list = malloc(1000*sizeof(Node));
	Node* root = NULL;		
	struct dirent* direntp;
	char name[NAME_MAX]; 
	int i = 0;
	int root_pid;

	memset(map, 0, 1024); // clear map to mark where tree branches are active
	root_pid = argc < 2 ? 1 : strtol(argv[1], NULL, 10); // use pid 1 as default


	// build list of each process
	while ( (direntp = readdir(dirp)) != NULL) {
		if (strtol(direntp->d_name, NULL, 10) == 0) continue;
		//overwrite the directory name after the /proc/ (6 bytes later)	
		strcpy(path + 6, direntp->d_name);
		strcat(path, "/status");
		pid_t ppid = get_ps(path, name);
		if (ppid != -1) {
			strcpy(ps_list[i].name, name);
			
			ps_list[i].pid = atoi(direntp->d_name); 
			ps_list[i].ppid = ppid;
			ps_list[i].child_count = 0;
			ps_list[i].children = malloc(sizeof(Node*)*MAX_CHILD);
			//for(int x=0; x < 20; x++)ps_list[i].children[x] = malloc(sizeof(Node));
			if (ps_list[i].children == NULL) {
				return -1;
			}
			
			i++;	
		}
	}
	closedir(dirp);
	// remove extra memeory 
	ps_list = realloc(ps_list, i*sizeof(Node));

	// build tree by finding parent of each node
	for (int p_ind = 0;  p_ind < i; p_ind++) {
		Node* child = &ps_list[p_ind];
		if(child->pid == root_pid) root = child;
		for (int pp_index = 0; pp_index < i; pp_index++) {
			Node* parent = &ps_list[pp_index];

			// connect parent and child
			if (child->ppid == parent->pid) {
				if (parent->child_count < MAX_CHILD) {
					parent->children[parent->child_count] = child;	
					parent->child_count ++;
				} 
				break;
			}
		}
	}

	char buf[32];
	sprintf(buf, "[%d %s]", root->pid, root->name);
	dfs(root, 1, 1, strlen(buf)+1);

	// free up dynamic memory
	for (int p_ind = 0; p_ind < i; p_ind++) {
		free(ps_list[p_ind].children);
	}
	free(ps_list);


}
