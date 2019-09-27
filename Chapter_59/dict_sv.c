/**
 * Simple dictionary that maps key/value pairs.
 * Uses inet websockets to communicate with cients.
 * Uses localhost and port 50000
 * See dict_cl.c for usage.
 *
 * Note: both the client and server demonstrate creating the 
 * their socets using the vanilla gaddrinfo vanilla API. This was 
 * so I could have some experience using with basics before relying on
 * more abstract interfaces.
 */
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include "readLineBuf.h"

#define MAX_KEY 10
#define MAX_VAL 100

/**
 * Smiple BST to map string keys to string values
 */
typedef struct Node {
	char *key;
	char *val;
	struct Node *left;
	struct Node *right;
} Node;

typedef Node *Map;

/**
 * Creates a simple key/value pair BST node
 */
static Node* newNode(char *key, char *val) {
	Node *n = malloc(sizeof(Node));
	n->key = malloc(MAX_KEY);
	n->val = malloc(MAX_VAL);
	strcpy(n->key, key);
	strcpy(n->val, val);
	n->left = NULL;
	n->right = NULL;
	return n;	
}


/**
 * Creates and adds a node specified by key and val to the BST
 * pointed to by root. Also provides modify capabilities by specifying the same
 * key as an already exisiting node.
 */
static Node* add(Node* root, char* key, char* val) {
	if (root == NULL)
		return newNode(key, val);
        int diff = strcmp(key, root->key); 
	if (diff == 0) {
		// replace value if it already exists
		strcpy(root->val, val);
		return root;
	} else if (diff < 0)
		root->left = add(root->left, key, val);
	else
		root->right = add(root->right, key, val);
	return root;
}

/**
 * Adds an already initialized subtree into the BST pointed to by root
 */
static Node* addTree(Node* root, Node *subtree) {
	if (root == NULL)
		return subtree;
	 
	if (strcmp(subtree->key, root->key) <= 0)
		root->left = addTree(root->left, subtree);
	else
		root->right = addTree(root->right, subtree);
	return root;
}

/**
 * Returns a pointer to a binary tree node
 */
static Node* get(Node* root, char* key) {
	if (root == NULL)
		return NULL;
	int diff = strcmp(key, root->key);

	if (diff == 0)
		return root;	
	else if (strcmp(key, root->key) < 0)
		return get(root->left, key);
	else
		return get(root->right, key);

}

/**
 *  Deletes node in a bst by adding the appropriate subtress into 
 *  its sibling tree
 */
static Node *del(Node *root, char *key) {
	if (root == NULL)
		return NULL;

	int diff = strcmp(key, root->key);
	if (diff == 0) {
		Node *tempR = root->right;	
		Node *tempL = root->left;	

		free(root->key);
		free(root->val);
		free(root);
		root->left = addTree(root->left, tempR);
		return root->left;
	}
	
	if (root->left && strcmp(key, root->left->key) == 0) {
		Node *tempR = root->left->right;	
		Node *tempL = root->left->left;	
		free(root->left->key);
		free(root->left->val);
		free(root->left);
		root->left = tempL;
		root->left = addTree(root->left, tempR);
		return root;
	} else if (root->right && strcmp(key, root->right->key) == 0) {
		Node *tempR = root->right->right;	
		Node *tempL = root->right->left;	
		free(root->right->key);
		free(root->right->val);
		free(root->right);
		root->right = tempR;
		root->right = addTree(root->right, tempL);
		return root;
	}

	if (diff < 0)
		root->left = del(root->left, key);
	else
		root->right = del(root->right, key);
		
	return root;
}


int main(int argc, char *argv[]) {
	struct addrinfo hints;
	struct addrinfo *result, *rp;
	char host[100], service[100];
	int sfd;
	Map map = NULL;

	// ingore sisgpipe from child closing
	signal(SIGPIPE, SIG_IGN);

	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_family = AF_UNSPEC; // ip4 or ip6
	hints.ai_socktype = SOCK_STREAM; // TCP
	hints.ai_flags = AI_PASSIVE; // wildcard ip for listening

	/* create listening address with a wilcard ip
	   this will let us accept TCP segments from any ip address */
	if (getaddrinfo(NULL, argc > 1 ? argv[1] : "50000", &hints, &result) != 0) {
		perror("getaddrinfo");
		return -1;
	}

	/* there may be multiple address that would work for our socket,
	   use the first one we find that binds successfully */
	for (rp = result; rp != NULL; rp = rp->ai_next) {
		sfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
		if (sfd == -1)
			continue;

		int optval = 1;
		if (setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) == -1) {
				perror("setsockopt");
				return -1;
		}
		// successfull bind 
		if (bind(sfd, rp->ai_addr, rp->ai_addrlen) == 0)
			break;

		// close on failure
		close(sfd);
	}
	if (rp == NULL) {
		fprintf(stderr, "Could not bind to a socket\n");
		return -1;
	}
	
	if (listen(sfd, 10) == -1){
		perror("listen");
		return -1;
	}

	getnameinfo(rp->ai_addr, rp->ai_addrlen, host, 100, service, 100, NI_NUMERICSERV);
	printf("Server listening on: %s:%s\n", host, service); 
	freeaddrinfo(result);

	/* Loop and iteratively handle client connections */
	for (;;) {
		int cfd;
		struct sockaddr_storage claddr; //works for both ipv4 and ipv6 
		char buf[256];
		struct rl rlbuf;
		char key[MAX_KEY];
		char val[MAX_VAL];
		Node* resp;

		socklen_t len = sizeof(struct sockaddr_storage);
		
		cfd = accept(sfd, (struct sockaddr *) &claddr, &len);
		if (cfd == -1) {
			if (errno == EINTR)
				continue; // continue on interrupt 
			perror("accept");
			return -1;
		}

		readLineBufInit(cfd, &rlbuf);
		while (readLineBuf(&rlbuf, buf, sizeof(buf)) > 0) {
		/* interpret a request by first charcter in the string 
			a: adds a value and key to the map
			g: retrieves a value 
			d: removes a value */
			switch (buf[0]) {
				case 'a':
					if (sscanf(buf, "a %s %s", key, val) != 2)
						continue;
					printf("adding %s %s\n", key, val);
					map = add(map, key, val);
					break;
				case 'g':
					if (sscanf(buf, "g %s", key) != 1)
						continue;
					printf("getting %s\n", key);
					resp = get(map, key);
					snprintf(val, MAX_VAL, "%s\n", resp ? resp->val : "");

					if (write(cfd, val, strlen(val)) != strlen(val))
						perror("write");
					break;
				case 'd':
					if (sscanf(buf, "d %s", key) != 1)
						continue;
					printf("deleting %s\n", key);
					map = del(map, key);
					break;
				default:
					break;
			}
		}
	
			
		if (close(cfd) == -1) {
			perror("close");
			return -1;
		}

	}
}
