#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>

typedef struct Node {
	pthread_mutex_t mtx;
	char key;
	void* val;
	struct Node* left;
	struct Node* right;
} Node;


Node* create_node(char key, void* val) {
	Node *leaf;
	leaf = malloc(sizeof(Node));
	leaf->key = key;
	leaf->val = val;
	leaf->left = NULL;
	leaf->right = NULL;
	pthread_mutex_init(&(leaf->mtx), NULL);	
	printf("created %c\n", key);
	return leaf;
}

void init(Node** tree) {
	*tree = create_node('\0', NULL);
}

void add (Node* tree, char key, void* val) {
	int done = 0; // use to skip loop and unlock mutexs

	if (tree == NULL) {
		printf("Root node must be initialized\n");
		return;
	}

	Node* cur = tree;
	Node* next;
	while (!done) {
		if(pthread_mutex_lock(&cur->mtx) != 0) {
			perror("lock");
			return;
		}

		if (cur->key == '\0') { // no value in root
			tree->key = key;
			tree->val = val;
			done = 1;
		} else if (key <= cur->key) {
			if (cur->left == NULL) {
				cur->left = create_node(key, val);
				done = 1;
			} else 
				next = cur->left;
		} else {
			if (cur->right == NULL) {
				cur->right = create_node(key, val); 
				done = 1;
			}  else
				next = cur->right;
		}
		if(pthread_mutex_unlock(&cur->mtx) != 0) {
			perror("unlock");
			return;
		}
		cur = next;
	}
}

int lookup (Node* tree, char key, void **value) {
	int done = 0;	
	int found = 0;	

	Node* cur = tree;
	Node* next;
	while(!done) {
		if(pthread_mutex_lock(&cur->mtx) != 0) {
			perror("lock");
			return -1;
		}
		if (cur->left == NULL && cur->right == NULL) {
			done = 1;
		}
		
		if (key == cur->key) {
			// point to val
			*value = cur->val;
			found = 1;
			done = 1;
		} else if (key <= cur->key) {
			if (cur->left == NULL) {
				done = 1;
			} else 
				next = cur->left;
		} else {
			if (cur->right == NULL) {
				done = 1;
			}  else
				next = cur->right;
		}
		if(pthread_mutex_unlock(&cur->mtx) != 0) {
			perror("unlock");
			return -1;
		}	
		cur = next;

	}

}

void printTree(Node* root) {
	if (root == NULL) 
		return;
	printf("here %c\n", root->key);
	printTree(root->left);
	printTree(root->right);
}

Node* t;
static void* threadFunc(void* arg) {
	char c  = *(char*)arg;
	void* ptr = malloc(1);;
	add(t, c, ptr);
	
	printf("%c done\n", c);
}
int main () {
       	init(&t);
	pthread_t thread_A, thread_B;
	char c = 'b';
	pthread_create(&thread_A, NULL, threadFunc, &c);
	char x = 'a';
	pthread_create(&thread_B, NULL, threadFunc, &x);

	char h = 'h';
	add(t, 'e', &h);
	void* ptr;

	pthread_join(thread_A, NULL);
	pthread_join(thread_B, NULL);
	lookup(t, 'e', &ptr); 
	printTree(t);	
}
