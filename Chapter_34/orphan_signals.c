#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

void handler(int signo) {
	// unsafe
	//
	FILE* fp = fopen("/dev/tty", "w");
	fprintf(fp, "delivered\n", 255);
	fclose(fp);
}

/**
 * Demonstrates how an orphaned process handle tty signals
 * Use h for handler, i for ignore and no argument for default
 */
int main (int argc, char* argv[]) {
	pid_t p = fork();
	char buf[1];
	struct sigaction sa;
	FILE* fp;	
	sa.sa_handler = handler;
	sa.sa_flags = SA_RESTART;

	sigaction(SIGTTIN, &sa, NULL);
	signal(SIGTTOU, SIG_DFL);
	switch(p) {
		case -1:
			perror("fork");
			break;
		case 0: 
			/* give children a chance to start and orphan 
			   this is a race condition, but will be fine 
			   for demo purposes */
			sleep(2);
			fp = fopen("/dev/tty", "w");
			fprintf(fp, "PPID: %ld\n", (long) getppid(), 10);
			if (argc == 1) { 
				// will exit after pause
				raise(SIGTTOU); // default
				pause();
			} else if (argv[1][0] == 'h') {
				raise(SIGTTIN);	// handled 
				pause();
			} else if (argv[1][0] == 'i') {
				raise(SIGTSTP); // ignored
				pause();
			}
			
			fprintf(fp, "done\n", 5);
			fclose(fp);
			break;
		default:
			exit(EXIT_SUCCESS);
	}
	
}
