#include <stdio.h>
#include <stddef.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

typedef struct block {
	int length;	
	struct block* next;
	struct block* prev;
} block;

//typedef unsigned long long int size_t;
void *fit(size_t new_len);
void pp(void * ptr);
void ppp();

//void** free_list;
void* head;
size_t min_size = sizeof(size_t) + 2*sizeof(void**);

void * m_malloc(size_t size) {
	// if not found request from kernel
	size = sizeof(size_t) + size;

	//printf("i"); 
	if (size < min_size) {
		size = min_size; // add space for next/prev
	}
	// search free list
	void* f = fit(size);
	void* new_mem;

	if (f != NULL) {
		new_mem = f;
	} else {

		new_mem= sbrk(size);
		if (new_mem != (void*)-1) {
			*((size_t*)new_mem) = size;
		}			
	}
	
	return new_mem + sizeof(size_t);	
}
void** get_prev(void* ptr) {
	return ptr + sizeof(size_t);
}
void** get_next(void* ptr) {
	return ptr + sizeof(void**) + sizeof(size_t);
}

void ppp() {
	printf("-----------------\n");
	//if (free_list != NULL) 
	for (void* cur = head; cur != NULL;) {
		
		pp(cur);
		cur = *get_next(cur);
	}
	printf("-----------------\n");

}


void m_free(void* ptr) {
	void* base = ptr - sizeof(size_t);
	//printf("allocated to: %lld\n", &ptr[7]-&base[7]);
	void** next = base + sizeof(size_t) + sizeof(void**);
	void** prev = base + sizeof(size_t);
	size_t length = *((size_t*)base);

	//if (free_list == NULL) {
		//free_list = sbrk(sizeof(void**));
	//}

	printf("freeing length = %lld\n", length);
	printf("head is %zu\n", head);
	printf("base: %zu (%s) next=%zu prev=%zu\n", base, ptr, *next, *prev);
	

	*prev = NULL; 
	*next = NULL;
	if (head != NULL) {
		// value at next = head of list
		*next = head;
		
		*(get_prev(head)) = base;
	} 
	// make head point to current 
	head = base; 
	head = base;


	printf("\tnext resides at %zu\tvalue in next: %zu\n", next, *next);
	printf("\tprev resides at %zu\tvalue in prev: %zu\n", prev, *prev);
	printf("\thead resides at %zu\tvalue in head: %zu\n", &head , head);
	printf("\n");
}

void pp(void *base) {
	void** next = base + sizeof(size_t) + sizeof(void**);
	void** prev = base + sizeof(size_t);
	size_t length = *((size_t*)base);
	printf("base: %zu (%lld) next=%zu prev=%zu\n", base, length, *next, *prev);

}


void* fit(size_t new_len) {
	//if (free_list == NULL) return NULL; 
	void* cur = head;
	printf("finding fit for %lld\n", new_len);
	 //traversal
	while(cur != NULL ) {
		size_t len = *((size_t*)cur );
		printf("found %zu\n", len);
		printf("%zu\n", cur);
		
		if (len == new_len) {
			// next of prev -> next of cur
			if (cur != head) { 
				*get_next(*get_prev(cur)) = *get_next(cur);  	
			} else {
				printf("test\n");	
				ppp(cur);
				head = *get_next(cur);
			}
			// prev of next -> prev of cur
			if (*get_next(cur) != NULL) {
				*get_prev(*get_next(cur)) = *get_prev(cur);		
			}
			*get_next(cur) = NULL;
			*get_prev(cur) = NULL;
			return cur;

		} else if (len >= min_size +new_len) {
			void* split = cur + new_len;
			// split
			*(size_t*)split = len - new_len;	
			*(size_t*)cur = new_len;

			// copy curs ptrs to split
			*get_prev(split) = *get_prev(cur);
			*get_next(split) = *get_next(cur);
			
			// prev of next -> split
			if (*get_next(split) != NULL) {
				*get_prev(*get_next(split)) = split;
			}
			
			// next of prev -> split	
			if (cur != head) {
				*get_next(*get_prev(cur)) = split;
			} else {
				head = split;
			}

			*get_next(cur) = NULL;
			*get_prev(cur) = NULL;

			printf("split done\t left: %lld  right: %lld\n", *(size_t*)cur, *(size_t*)split);
			return cur;
		}
		cur = *(get_next(cur)); 
	}
	printf("no fit\n");
	return NULL;
}


int main() {
	char* i = m_malloc(4);
	char* k = m_malloc(32);
	//printf("allocated to: %lld\n", *(size_t*)(i - sizeof(size_t)));
	////printf("allocated to: %lld\n", *(size_t*)(j - sizeof(size_t)));
	char* j = m_malloc(10000);

	if (i == NULL) {
		printf("malloc failed\n");
		strerror(errno);
		return -1;
	}	
	ppp();
	m_free(j);
	m_free(k);
	m_free(i);
	char * x = m_malloc(500);
	m_free(x);
	ppp();
	//ppp();	
	return 0;
}
