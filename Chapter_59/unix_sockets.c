#include "unix_sockets.h" 

/**
 * Creates a unix domain socket and binds an address to path
 */
int unBind(const char* path, int type) {
	struct sockaddr_un addr;
	int fd;

	fd = socket(AF_UNIX, type, 0);
	if (fd == -1)
		return -1;

	if (strlen(path) > sizeof(addr.sun_path) - 1) {
		errno = E2BIG;
		return -1;
	}

	memset(&addr, 0, sizeof(struct sockaddr_un));
	addr.sun_family = AF_UNIX;

	if (path[0] == '\0')
		strncpy(&addr.sun_path[1], &path[1], sizeof(addr.sun_path) - 1);
	else 
		strncpy(addr.sun_path, path, sizeof(addr.sun_path));
	

	// bind this address to the to the fd socket so we can listen
	if (bind(fd, (struct sockaddr *) &addr, sizeof(struct sockaddr_un)) == -1)
		return -1;

	return fd;
}

/**
 * Creates a unix domain socket then binds and listens to the socket 
 * located at path
 */
int unListen(const char *path, int backlog) {
	int sfd;

	sfd = unBind(path, SOCK_STREAM);
	if (sfd == -1)
		return -1;

	if (listen(sfd, backlog) == -1)
		return -1;
	
	return sfd;
}

/**
 *  Creates a socket and connects it to the server at path
 */
int unConnect(const char* path, int type) {
	int cfd;
	struct sockaddr_un addr;

	cfd = socket(AF_UNIX, type, 0);
	if (cfd == -1)
		return -1;

	if (strlen(path) > sizeof(addr.sun_path) - 1) {
		errno = E2BIG;
		return -1;
	}

	if (remove(path) == -1 && errno != ENOENT)
		return -1;

	memset(&addr, 0, sizeof(struct sockaddr_un));
	addr.sun_family = AF_UNIX;

	if (path[0] == '\0')
		strncpy(&addr.sun_path[1], &path[1], sizeof(addr.sun_path) - 1);
	else 
		strncpy(addr.sun_path, path, sizeof(addr.sun_path));

	if (connect(cfd, (struct sockaddr *) &addr,
				sizeof(struct sockaddr_un)) == -1)
		return -1;
	return cfd;
}

