#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern char** environ;


int setenv(const char* name, const char* value, int overwrite);
int unsetenv(const char* name);

int setenv(const char* name, const char* value, int overwrite) {


}

int unsetenv(const char* name) {
	int i = 0; 
	char** p;
	char var[strlen(name)+2];

	strcpy(var, name);
	strcat(var, "=");
		
	//for (p = environ; *p != NULL;) {
		//if (strncmp(*p, var, strlen(var)) == 0 && (*p)[strlen(var)-1] == '=') { // found var
			//printf("found %s\n", *p);
			//for (char** sp = p; *sp != NULL; sp++)
				//*sp = *(sp + 1); // make sucessive pointer hold address of next string
		//} else {
			//p++;
		//}
	//}
	

	// O(n) solution where n is the number of duplicates, previous solution had to iterate for each 
	// instance while this solution only requires a swap for each hole
	for(p = environ; *p != NULL; p++);
	p--; // refrence to the last valid path
	
	// handle the case where the first string is also the last	
	if (p == environ) {
		*p = NULL;	
	}

	char ** cur;
	for (cur = environ; *cur != NULL; cur++) {
		if (strncmp(*cur, var, strlen(var)) == 0 && (*cur)[strlen(var)-1] == '=') { // found var 
			printf("found %s\n", *cur);
			*cur = *p; // store last string's addr in the current pointer
			*p = NULL; // make the last address store NULL
			p--;	// last adress has shifted down
		}
	}
	
	return 0;
}

int main() {
	printf("%s\n", getenv("PATH"));
	unsetenv("PATH");
	unsetenv("USER");
	printf("\n");
	for (char** p = environ; *p != NULL; p++) printf("%s\n", *p);
	//printf("%s\n", getenv("PATH"));
	return 0;
}
