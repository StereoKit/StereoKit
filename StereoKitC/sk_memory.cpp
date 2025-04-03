#include "sk_memory.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

namespace sk {

#if !defined(SK_DEBUG_MEM)

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

void *sk_calloc(size_t bytes) {
	void *result = calloc(bytes, 1);
	if (result == nullptr && bytes > 0) {
		fprintf(stderr, "Memory alloc failed!");
		abort();
	}
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

///////////////////////////////////////////

void _sk_free(void* memory) {
	free(memory);
}

///////////////////////////////////////////

void sk_mem_log_allocations() {
}

#else

struct mem_info_t {
	uint32_t    marker;
	size_t      bytes;
	const char* type;
	const char* filename;
	int32_t     line;
	mem_info_t* next;
	mem_info_t* prev;
};
mem_info_t* mem_tracker_root = nullptr;
mem_info_t* mem_tracker_tail = nullptr;

///////////////////////////////////////////

void *sk_malloc_d(size_t bytes, const char* type, const char* filename, int32_t line) {
	void *result = malloc(bytes + sizeof(mem_info_t));
	if (result == nullptr && bytes > 0) {
		fprintf(stderr, "Memory alloc failed!");
		abort();
	}

	// This can be a really handy way to debug a specific allocation!
	//if (bytes == 13851053 && line == 182) {
	//	printf("break\n");
	//}

	mem_info_t* info = (mem_info_t*)result;
	*info = {};
	info->marker   = 0x12345678;
	info->bytes    = bytes;
	info->filename = filename;
	info->line     = line;
	info->type     = type;
	if (mem_tracker_root == nullptr) { mem_tracker_root = mem_tracker_tail = info; }
	else                             { mem_tracker_tail->next = info; info->prev = mem_tracker_tail; mem_tracker_tail = info; }
	return ((uint8_t*)result)+sizeof(mem_info_t);
}

///////////////////////////////////////////

void *sk_calloc_d(size_t bytes, const char* type, const char* filename, int32_t line) {
	void *result = calloc(bytes + sizeof(mem_info_t), 1);
	if (result == nullptr && bytes > 0) {
		fprintf(stderr, "Memory alloc failed!");
		abort();
	}
	
	mem_info_t* info = (mem_info_t*)result;
	*info = {};
	info->marker   = 0x12345678;
	info->bytes    = bytes;
	info->filename = filename;
	info->line     = line;
	info->type     = type;
	if (mem_tracker_root == nullptr) { mem_tracker_root = mem_tracker_tail = info; }
	else                             { mem_tracker_tail->next = info; info->prev = mem_tracker_tail; mem_tracker_tail = info; }
	return ((uint8_t*)result)+sizeof(mem_info_t);
}

///////////////////////////////////////////

void *sk_realloc_d(void *memory, size_t bytes, const char* type, const char* filename, int32_t line) {
	if (memory == nullptr) return sk_malloc_d(bytes, type, filename, line);

	mem_info_t* prev_info = (mem_info_t*)(((uint8_t*)memory) - sizeof(mem_info_t));
	void *result = realloc(prev_info, bytes + sizeof(mem_info_t));
	if (result == nullptr && bytes > 0) {
		fprintf(stderr, "Memory alloc failed!");
		abort();
	}

	mem_info_t* info = (mem_info_t*)result;
	info->marker   = 0x12345678;
	info->bytes    = bytes;
	info->filename = filename;
	info->line     = line;
	info->type     = type;
	if (info->prev != nullptr) info->prev->next = info;
	if (info->next != nullptr) info->next->prev = info;
	if (mem_tracker_tail == prev_info) mem_tracker_tail = info;
	if (mem_tracker_root == prev_info) mem_tracker_root = info;
	return ((uint8_t*)result)+sizeof(mem_info_t);
}

///////////////////////////////////////////

void _sk_free_d(void* memory, const char* filename, int32_t line) {
	if (memory == nullptr) return;

	mem_info_t* info = (mem_info_t*)(((uint8_t*)memory) - sizeof(mem_info_t));
	if (info->marker != 0x12345678) {
		fprintf(stderr, "Memory free failed! Memory was not allocated by sk_malloc, sk_calloc, or sk_realloc!");
		abort();
	}
	if (info->prev != nullptr) info->prev->next = info->next;
	if (info->next != nullptr) info->next->prev = info->prev;
	if (mem_tracker_tail == info) mem_tracker_tail = info->prev;
	if (mem_tracker_root == info) mem_tracker_root = info->next;

	free(info);
}

///////////////////////////////////////////

void sk_mem_log_allocations() {
	mem_info_t* mem = mem_tracker_root;
	size_t  total = 0;
	size_t  max   = 0;
	int32_t count = 0;
	while (mem != nullptr) {
		const char *filename = strrchr(mem->filename, '/');
		if (filename == nullptr) filename = strrchr(mem->filename, '\\');
		if (filename == nullptr) filename = mem->filename;
		else                     filename += 1;

		total += mem->bytes;
		if (max < mem->bytes)
			max = mem->bytes;
		count += 1;

		printf("%s #%d - %s - %d", filename, mem->line, mem->type, (int32_t)mem->bytes);
		if (strcmp(filename, "stref.cpp") == 0)
			printf(" - %s\n", mem + 1);
		else
			printf("\n");

		mem = mem->next;
	}
	printf("%d allocations, total memory: %.2fmb, largest allocation: %.2fmb\n", count, total/(1024.0f * 1024.0f), max/(1024.0f * 1024.0f));
}
#endif

} // namespace sk