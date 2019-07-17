#include <stdio.h>
#include <stdlib.h>
#include <setjmp.h>

// program to test long jump effect into already returned function
static jmp_buf env;

static void f() {
	longjmp(env, 1);
	printf("finished second function\n");
}
static int to_finish() {
	volatile int i = 10;
	if (setjmp(env) == 0) { //normal flow
	}

	printf("%d\n", i);
	printf("finished 'to_finish' function\n");
}

int main() {
	to_finish();
	printf("to finish has returned\n");
	f();
}
