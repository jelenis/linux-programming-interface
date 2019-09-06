#include <syslog.h>
#include <stdio.h>
#include <stdlib.h>

/**
 * Creates a log, like logger(1).
 * Check /etc/syslog.conf for machines without systemd
 * If your machine is runinng systemd, check journalctl(1)
 */
int main(int argc, char* argv[]) {
	int priority = LOG_USER;
	if (argc < 2) {
		printf("Please sepecify a message\n");
		printf("LOG_EMERG\t%d\n", LOG_EMERG);
		printf("LOG_ALERT\t%d\n", LOG_EMERG);
		printf("LOG_CRIT\t%d\n", LOG_CRIT);
		printf("LOG_ERR\t\t%d\n", LOG_ERR);
		printf("LOG_WARNING\t%d\n", LOG_WARNING);
		printf("LOG_NOTICE\t%d\n", LOG_NOTICE);
		printf("LOG_INFO\t%d\n", LOG_INFO);
		printf("LOG_DEBUG\t%d\n", LOG_DEBUG);
		return -1;
	}
	if (argc > 2)
		priority |= strtol(argv[2], NULL, 10);


	syslog(priority, "%s", argv[1]);

}
