#pragma once

#include <malloc.h>

namespace sk {

// Safer memory allocation functions, will kill the app on failure.
void *sk_malloc     (              size_t bytes);
void *sk_calloc     (              size_t bytes);
void *sk_realloc    (void *memory, size_t bytes);
void  _sk_free      (void *memory);

#define sk_free(memory) { _sk_free(memory); memory = nullptr; };

#define sk_malloc_t(T, count) ((T*)sk_malloc ((count) * sizeof(T)))
#define sk_calloc_t(T, count) ((T*)sk_calloc((count) * sizeof(T)))
#define sk_realloc_t(T, memory, count) ((T*)sk_realloc(memory, (count) * sizeof(T)))

} // namespace sk