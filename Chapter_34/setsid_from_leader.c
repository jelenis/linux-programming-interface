#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
/**
 * Demonstrates that a process group leader cannot call setsid.
 */
int main (int argc, char* argv[]) {
	// become process group leader
	setpgid(0, 0);

	// attempt to call become session process
	if (setsid() == -1) {
		perror("setsid");
		return -1;
	}
	
	return 0;	
}
