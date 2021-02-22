#pragma once

#include <malloc.h>

namespace sk {

// Safer memory allocation functions, will kill the app on failure.
void *sk_malloc (              size_t bytes);
void *sk_realloc(void *memory, size_t bytes);

template<typename T> inline T *sk_malloc_t (              size_t count) { return (T*)sk_malloc (        count * sizeof(T)); }
template<typename T> inline T *sk_realloc_t(void *memory, size_t count) { return (T*)sk_realloc(memory, count * sizeof(T)); }

} // namespace sk