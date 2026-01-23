#include "sk_memory.h"
#include "libraries/profiler.h"

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
	profiler_alloc(result, bytes);
	return result;
}

///////////////////////////////////////////

void *sk_calloc(size_t bytes) {
	void *result = calloc(bytes, 1);
	if (result == nullptr && bytes > 0) {
		fprintf(stderr, "Memory alloc failed!");
		abort();
	}
	profiler_alloc(result, bytes);
	return result;
}

///////////////////////////////////////////

void *sk_realloc(void *memory, size_t bytes) {
	if (memory) {
		profiler_free(memory);
	}
	void *result = realloc(memory, bytes);
	if (result == nullptr && bytes > 0) {
		fprintf(stderr, "Memory alloc failed!");
		abort();
	}
	profiler_alloc(result, bytes);
	return result;
}

///////////////////////////////////////////

void _sk_free(void* memory) {
	if (memory) {
		profiler_free(memory);
	}
	free(memory);
}

///////////////////////////////////////////

void sk_mem_log_allocations() {
}

#else

struct mem_info_t {
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
	mem_info_t* mem = mem_tracker_root;
	while (mem != nullptr) {
		if (mem == result) {
			fprintf(stderr, "Memory allocation is already tracked!");
			abort();
		}
		if ((uint8_t*)mem <= ((uint8_t*)result + bytes + sizeof(mem_info_t)) && (uint8_t*)result <= ((uint8_t*)mem + mem->bytes + sizeof(mem_info_t))) {
			fprintf(stderr, "Overlapping tracked allocations!");
			abort();
		}
		mem = mem->next;
	}
	profiler_alloc(((uint8_t*)result) + sizeof(mem_info_t), bytes);

	// This can be a really handy way to debug a specific allocation!
	//if (bytes == 13851053 && line == 182) {
	//	printf("break\n");
	//}

	mem_info_t* info = (mem_info_t*)result;
	*info = {};
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
	mem_info_t* mem = mem_tracker_root;
	while (mem != nullptr) {
		if (mem == result) {
			fprintf(stderr, "Memory allocation is already tracked!");
			abort();
		}
		if ((uint8_t*)mem <= ((uint8_t*)result + bytes + sizeof(mem_info_t)) && (uint8_t*)result <= ((uint8_t*)mem + mem->bytes + sizeof(mem_info_t))) {
			fprintf(stderr, "Overlapping tracked allocations!");
			abort();
		}
		mem = mem->next;
	}
	profiler_alloc(((uint8_t*)result) + sizeof(mem_info_t), bytes);
	
	mem_info_t* info = (mem_info_t*)result;
	*info = {};
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
	profiler_free(memory);
	void *result = realloc(prev_info, bytes + sizeof(mem_info_t));
	if (result == nullptr && bytes > 0) {
		fprintf(stderr, "Memory alloc failed!");
		abort();
	}
	mem_info_t* mem = mem_tracker_root;
	while (mem != nullptr) {
		if (mem == result) {
			fprintf(stderr, "Memory allocation is already tracked!");
			abort();
		}
		if ((uint8_t*)mem <= ((uint8_t*)result + bytes + sizeof(mem_info_t)) && (uint8_t*)result <= ((uint8_t*)mem + mem->bytes + sizeof(mem_info_t))) {
			fprintf(stderr, "Overlapping tracked allocations!");
			abort();
		}
		mem = mem->next;
	}
	profiler_alloc(((uint8_t*)result) + sizeof(mem_info_t), bytes);
	
	mem_info_t* info = (mem_info_t*)result;
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

	mem_info_t* mem = mem_tracker_root;
	bool found = false;
	while (mem != nullptr) {
		if (mem == info) {
			found = true;
			break;
		}
		mem = mem->next;
	}
	if (!found) {
		fprintf(stderr, "Attempted to free memory that wasn't allocated by us!");
		abort();
	}

	profiler_free(memory);

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