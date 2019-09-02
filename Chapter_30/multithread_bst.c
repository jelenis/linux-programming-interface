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

typedef struct Tree {
	Node* root;
	pthread_mutex_t mtx;
} Tree;
Tree tre;

void initialize(Tree* tre) {
	tre->root = NULL;
	pthread_mutex_init(&(tre->mtx), NULL);	
}

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


int lookup (Tree* t, char key, void **value) {
	int done = 0;	
	int found = 0;	

	Node* cur = t->root;
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
	if (root == NULL) {
		printf("(null)\n");
		return;
	}
	printf("here %c\n", root->key);
	printTree(root->left);
	printTree(root->right);
}

void add(Tree* t, char key, void* val) {
	int done = 0; // use to skip loop and unlock mutexs

	if (t->root == NULL) {
		t->root = create_node(key, val);
		return;
	}

	Node* cur = t->root;
	Node* next;
	while (!done) {
		if(pthread_mutex_lock(&cur->mtx) != 0) {
			perror("lock");
			return;
		}

		if (key <= cur->key) {
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

void insert_right(Node* root, Node* leaf) {
	if (leaf == NULL) {
		return;	
	}

	Node* cur = root;
	while (cur && cur->right) {
		if (pthread_mutex_lock(&cur->mtx) != 0) {
			perror("lock");
			return;
		}		
		cur = cur->right;
		if(pthread_mutex_unlock(&cur->mtx) != 0) {
			perror("unlock");
			return;
		}	
	}
	cur->right = leaf;


}
void delete(Tree* tree, char key) {
	Node* cur = tree->root;
	Node* next;
	Node* prev;	
	int done = 0;


	while (cur && !done) {
		if(pthread_mutex_lock(&cur->mtx) != 0) {
			perror("lock");
			return;
		}	

		if (key == cur->key) {
			if (prev == NULL) { // root
				if(pthread_mutex_lock(&tree->mtx) != 0) {
					perror("lock");
					return;
				}		
				insert_right(cur->left, cur->right);
				tree->root = cur->left;
				if(pthread_mutex_unlock(&tree->mtx) != 0) {
					perror("unlock");
					return;
				}		
				done = 1;
			} else if (prev->left == cur) { // left subchild
				if (cur->left) {
					prev->left = cur->left;
					insert_right(cur->left, cur->right);
				} else 
					prev->left = cur->right;
			} else {
				if (cur->left) {
					prev->right = cur->left;
					insert_right(cur->left, cur->right);
				} else 
					prev->right = cur->right;
			}
			
			
			printf("found %c\n", cur->key);
			done = 1;
		} else if (key <= cur->key && cur->left != NULL) {
			next = cur->left;
		} else if (cur->right != NULL) {
			next = cur->right;
		} else {
			done = 1;
		}
		if(prev && pthread_mutex_unlock(&prev->mtx) != 0) {
			perror("unlock");
			return;
		}
		prev = cur;
		cur = next;

		
	}
	if(pthread_mutex_unlock(&cur->mtx) != 0) {
		perror("unlock");
		return;
	}


}
static void* threadFunc(void* arg) {
	char c  = *(char*)arg;
	void* ptr = malloc(1);;
	add(&tre, c, ptr);

	printf("%c done\n", c);
}

int main () {
	pthread_t thread_A, thread_B;
	initialize(&tre);
	
	// testing chars as keys
	char b = 'b';
	char z = 'z';
	add(&tre, 'e', NULL);

	pthread_create(&thread_A, NULL, threadFunc, &b);
	pthread_create(&thread_B, NULL, threadFunc, &z);

	pthread_join(thread_A, NULL);
	pthread_join(thread_B, NULL);
	add(&tre, 'c', NULL);

	// print the tree before 
	printTree(tre.root);	
	delete(&tre, 'z');
	printf("**** After Delete ****\n");
	// print the tree after deleting
	printTree(tre.root);	
}
