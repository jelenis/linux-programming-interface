/**
 * Simple multi-threaded BST implementation. Uses functions specified by lpi
 */
#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>

/**
 * BST node, with mutex locks for multi-threading
 */
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

Tree tre; // Tree stucture containing pointer to root of BST

/**
 * Creates a Tree structure
 */
void initialize(Tree* tre) {
	tre->root = NULL;
	pthread_mutex_init(&(tre->mtx), NULL);	
}

/**
 * Function to easily allocate and create BST node
 */
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

/**
 * Returns the value of a node in the pointer pointed to by value.
 */
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


/**
 * inserts a node into the BST into the BST pointed to by t
 */
void add(Tree* t, char key, void* val) {
	int done = 0; // use to skip loop and unlock mutexs

	if (t->root == NULL) {
		/* lock the tree so that other root nodes are not 
		added at the same time */
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

/**
 * Inserts the subtree pointed to by leaf at the next poistion in the tree
 * pointed to by root
 *
 */
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

/**
 * Deletes a node in tree in tree if the nodes key matches key
 */
void delete(Tree* tree, char key) {
	Node* cur = tree->root;
	Node* next;
	Node* prev = NULL;	
	int done = 0; // use flag to exit loop and clean up locks

	/* iterate throught the BST keeping a reference to the previous node.
	 Once when a node is visited it is locked. The node is unlocked when 
	 the reference to the previous node is updated */
	while (cur && !done) {
		// lock this node until prev no longer holds a reference 
		if(pthread_mutex_lock(&cur->mtx) != 0) {
			perror("lock");
			return;
		}	

		if (key == cur->key) {
			if (prev == NULL) { 
				// lock the tree remove root 
				if(pthread_mutex_lock(&tree->mtx) != 0) {
					perror("lock");
					return;
				}		
				/* place the right sub-tree in the next
				 available position after the left subtree */
				insert_right(cur->left, cur->right);

				// make left subtree the root
				tree->root = cur->left; 				
				if(pthread_mutex_unlock(&tree->mtx) != 0) {
					perror("unlock");
					return;
				}		
				done = 1; // clean up other locks
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
			done = 1;
		} else if (key <= cur->key && cur->left != NULL) { 
			// key is on left
			next = cur->left;
		} else if (cur->right != NULL) {
		       	// key is on right
			next = cur->right;
		} else {
			// could not locate key
			done = 1;
		}
		// unlock the previous node if it exists
		if(prev && pthread_mutex_unlock(&prev->mtx) != 0) {
			perror("unlock");
			return;
		}
		prev = cur;
		cur = next;

		
	}
	// current mutex is still locked, unlock it
	if(pthread_mutex_unlock(&cur->mtx) != 0) {
		perror("unlock");
		return;
	}


}

/**
 * Function run seperate threads, part of driver program to test multi-thread
 * BST
 */
static void* threadFunc(void* arg) {
	char c  = *(char*)arg;
	void* ptr = malloc(1);;
	add(&tre, c, ptr);

	printf("%c done\n", c);
}

/**
 * Prints a BST in preorder pointed to by root
 */
void printTree(Node* root) {
	if (root == NULL) {
		printf("(null)\n");
		return;
	}
	printf("here %c\n", root->key);
	printTree(root->left);
	printTree(root->right);
}

/**
 * Driver program to test multi-thread BST
 */
int main () {
	pthread_t thread_A, thread_B;
	initialize(&tre);
	
	// testing chars as keys
	char b = 'b';
	char z = 'z';
	add(&tre, 'e', NULL);

	// run two threads to asynchronously add to BST
	pthread_create(&thread_A, NULL, threadFunc, &b);
	pthread_create(&thread_B, NULL, threadFunc, &z);

	pthread_join(thread_A, NULL);
	pthread_join(thread_B, NULL);
	add(&tre, 'c', NULL);

	// print the tree before 
	printTree(tre.root);	
	delete(&tre, 'e'); // delete a node
	printf("**** After Delete ****\n");
	// print the tree after deleting
	printTree(tre.root);	
}
