#include <stdio.h>
#include <malloc.h>
#include <stdlib.h>
#include <string.h>

namespace sk {

///////////////////////////////////////////

void *sk_malloc(size_t bytes) {
	void *result = malloc(bytes);
	if (result == nullptr && bytes > 0) {
		fprintf(stderr, "Memory alloc failed!");
		abort();
	}
	return result;
}

///////////////////////////////////////////

void *sk_malloc_zero(size_t bytes) {
	void *result = malloc(bytes);
	if (result == nullptr && bytes > 0) {
		fprintf(stderr, "Memory alloc failed!");
		abort();
	}
	memset(result, 0, bytes);
	return result;
}

///////////////////////////////////////////

void *sk_realloc(void *memory, size_t bytes) {
	void *result = realloc(memory, bytes);
	if (result == nullptr && bytes > 0) {
		fprintf(stderr, "Memory alloc failed!");
		abort();
	}
	return result;
}

} // namespace sk