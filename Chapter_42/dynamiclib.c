#include <stdio.h>
#include <dlfcn.h>
#include <gnu/lib-names.h>

/**
 * Demonstrates that dlclose does not unload the library
 * if there is still a reference to one of its symbols in another
 * library
 */
int main () {
	// load the math library and dependent vector math library
	void* math_handle = dlopen(LIBM_SO, RTLD_LAZY);	
	void* vec_handle = dlopen(LIBMVEC_SO, RTLD_LAZY);	

	double (*cosine)(double); // fcn ptr -> double cosine(double) {}
	if (!math_handle) {
		printf("%s\n", dlerror());
		return -1;
	}
	dlerror(); // clear exitsing errors

	/* ISO C forbids function ptr = (void*)
		i.e. cosine = dlsym(handle, "cos");
	  cast function ptr to a (void**) and dereference it so that we are
	  setting a (void*) = (void*)  this avoids the warning*/
	*(void**)&cosine =  dlsym(math_handle, "cos");
	
	// will remain open since libmvec still has reference
	dlclose(math_handle); 

	// can still use math library
	printf("cosine(0) = %.2lf\n", cosine(0));

	dlclose(vec_handle); 
}
