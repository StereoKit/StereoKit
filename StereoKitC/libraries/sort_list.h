#pragma once

#include <stdint.h>
#include <malloc.h>
#include <string.h>

#define sl_free(sl_list) _sl_free((void**)&sl_list)
#define sl_indexof(sl_list, ui64_key) _sl_indexof(sl_list, ui64_key)
#define sl_insert(sl_list, ui32_index, ui64_key, T_item) _sl_insert((void**)&sl_list, sizeof(*sl_list), ui32_index, ui64_key, &T_item)

int32_t _sl_indexof(void *list, uint64_t key);
int32_t _sl_insert (void **list, size_t el_size, int32_t index, uint64_t key, void *item);
void    _sl_free   (void **list);

#ifdef SL_IMPLEMENTATION

struct _sl_header_t {
	uint32_t  cap;
	uint32_t  count;
	uint64_t *hash_table;
};

inline _sl_header_t *_sl_get_header(void *list) {
	return ((_sl_header_t *)(list)-1);
}

int32_t _sl_indexof(void *list, uint64_t key) {
	if (list == nullptr) return -1;
	_sl_header_t *head = _sl_get_header(list);

	// binary search
	int32_t l = 0;
	int32_t r = head->count - 1;
	while (l <= r) {
		int32_t mid = (l+r) / 2;
		if      (head->hash_table[mid] < key) l = mid + 1;
		else if (head->hash_table[mid] > key) r = mid - 1;
		else                                  return mid;
	}
	return r < 0 ? r : -(r+2);
}

void _sl_grow(void **list, size_t el_size, uint32_t add_capacity) {
	_sl_header_t *header = nullptr;
	if (*list == nullptr) {
		add_capacity = add_capacity < 2 ? 2 : add_capacity;
		header             = (_sl_header_t*)malloc(sizeof(_sl_header_t) + el_size*add_capacity);
		header->hash_table = (uint64_t    *)malloc(sizeof(uint64_t) * add_capacity);
		header->count      = 0;
		header->cap        = add_capacity;
		*list = header + 1;
	} else {
		header = _sl_get_header(*list);
		if (header->count + add_capacity > header->cap) {
			if (add_capacity < header->cap)
				add_capacity = header->cap;
			add_capacity += header->cap;

			uint64_t *hash_table = (uint64_t*)realloc(header->hash_table, sizeof(uint64_t) * add_capacity);
			header = (_sl_header_t*)realloc(header, sizeof(_sl_header_t) + el_size * add_capacity);
			header->hash_table = hash_table;
			header->cap        = add_capacity;
			*list = header + 1;
		}
	}
}

int32_t _sl_insert(void **list, size_t el_size, int32_t index, uint64_t key, void *item) {
	_sl_grow(list, el_size, 1);
	_sl_header_t *head = _sl_get_header(*list);

	if (index < 0) index = -(index+1);
	if (index >= head->count) {
		index = head->count;
	}
	uint8_t  *at      = (uint8_t *)(*list) + el_size * index;
	uint64_t *hash_at = &head->hash_table[index];
	if (index != head->count) {
		memmove(at + el_size, at,      el_size * (head->count - index));
		memmove(hash_at + 1,  hash_at, sizeof(uint64_t) * (head->count - index));
	}
	memcpy(at, item, el_size);
	*hash_at = key;
	head->count += 1;
	return index;
}

void _sl_free(void **list) {
	if (*list == nullptr) return;
	_sl_header_t *head = _sl_get_header(*list);
	free(head->hash_table);
	free(head);
	*list = nullptr;
}

#endif