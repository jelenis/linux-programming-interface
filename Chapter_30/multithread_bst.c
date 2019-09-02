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
} Tree;

void init (Node* tree) {
	tree->left = NULL;
	tree->right = NULL;
	pthread_mutex_init(&tree->mtx, NULL);	
}

void add (Node* tree, char key, void* val) {
	if (tree == NULL) {

		return;
	}
	Node* cur = tree;

	while (1) {
		if (key <= cur->key) {
			if (cur->left == NULL) {
				cur->left = malloc(sizeof(Node));
				init(cur->left);
				cur->left->key = key;
				cur->left->val = val;
				break;			
			} 
			cur = cur->left;
		} else {
			if (cur->right == NULL) {
				cur->right = malloc(sizeof(Node));
				init(cur->right);
				cur->right->key = key;
				cur->right->val = val;
				break;
			} 
			cur = cur->right;
		}
	}
}
int lookup (char* key, void **value) {
	if 
}

void printTree(Node* root) {
	if (root == NULL) 
		return;
	printf("here %c\n", root->key);
	printTree(root->left);
	printTree(root->right);
}

int main () {
	Node* tree = malloc(sizeof(Node));
	init(tree);
	tree->key = 'd';
	
	add(tree, 'b', NULL);
	add(tree, 'e', NULL);

	printTree(tree);	
}
