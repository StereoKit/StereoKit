// SPDX-License-Identifier: MIT
// The authors below grant copyright rights under the MIT license:
// Copyright (c) 2019-2023 Nick Klingensmith
// Copyright (c) 2023 Qualcomm Technologies, Inc.

#pragma once

#include <stdint.h>
#include <stddef.h>

namespace sk {

#if !defined(SK_DEBUG_MEM)

// Safer memory allocation functions, will kill the app on failure.
void *sk_malloc     (              size_t bytes);
void *sk_calloc     (              size_t bytes);
void *sk_realloc    (void *memory, size_t bytes);
void  _sk_free      (void *memory);

#define sk_free(memory) { _sk_free(memory); memory = nullptr; };

#define sk_malloc_t(T, count) ((T*)sk_malloc ((count) * sizeof(T)))
#define sk_malloc_zero_t(T, count) ((T*)sk_calloc((count) * sizeof(T)))
#define sk_realloc_t(T, memory, count) ((T*)sk_realloc(memory, (count) * sizeof(T)))

#else

// Safer memory allocation functions, will kill the app on failure.
void *sk_malloc_d     (              size_t bytes, const char *type, const char *filename, int32_t line);
void *sk_calloc_d     (              size_t bytes, const char *type, const char* filename, int32_t line);
void *sk_realloc_d    (void *memory, size_t bytes, const char *type, const char* filename, int32_t line);
void  _sk_free_d      (void *memory, const char* filename, int32_t line);

#define sk_malloc(bytes) sk_malloc_d(bytes, "raw", __FILE__, __LINE__)
#define sk_calloc(bytes) sk_calloc_d(bytes, "raw", __FILE__, __LINE__)
#define sk_realloc(memory, bytes) sk_realloc_d(memory, bytes, "raw", __FILE__, __LINE__)
#define _sk_free(memory) _sk_free_d(memory, __FILE__, __LINE__)

#define sk_free(memory) { _sk_free_d(memory, __FILE__, __LINE__); memory = nullptr; };

#define sk_malloc_t(T, count) ((T*)sk_malloc_d((count) * sizeof(T), #T, __FILE__, __LINE__))
#define sk_calloc_t(T, count) ((T*)sk_calloc_d((count) * sizeof(T), #T, __FILE__, __LINE__))
#define sk_realloc_t(T, memory, count) ((T*)sk_realloc_d(memory, (count) * sizeof(T), #T, __FILE__, __LINE__))

#endif

void sk_mem_log_allocations();

} // namespace sk