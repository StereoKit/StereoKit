/*Licensed under MIT or Public Domain. See bottom of file for details.

array.h

	array.h is a dynamic array implementation using Plain Old Data structs. 

	array_t handles memory allocation and resizing, but follows standard and
	predictable behavior during argument passing, construction, etc. Memory 
	must be freed explicity with .free(), as it doesn't use constructors or 
	destructors.

	array_view_t is a partial 'view' of a chunk of memory that contains much 
	more than just what we're interested in. For example, if we want to 
	interact with only the 'y' component of an array of vec3s. It also has 
	tools for de-interlacing data from such chunks of memory. While array_t 
	should be better for most use cases, this is particularly handy for data 
	you don't know the whole story about, or when loading data from files.

Example usage:

	array_t<vec3> vertices = {};
	vertices.resize(3); // resize is optional

	vertices.add( vec3{1,0,0} );
	vertices.add( vec3{0,1,0} );
	vertices.add( vec3{0,0,1} );

	// Sort in ascending order using the y component
	vertices.sort<vec3, float, &vec3::y>();

	// Binary search y components for 0.5, binary_search only works on sorted
	// lists.
	int32_t at = vertices.binary_search(&vec3::y, 0.5f);

	// If result is positive, it's the index of the item. If negative, then
	// the '~' operator gives you the insert index for that value.
	if (at < 0) at = ~at;
	vertices.insert(at, vec3{0, 0.5f, 0});

	// standard array access
	for (int32_t i=0; i<vertices.count; i+=1) {
		vertices[i] += vec3{1,1,1};
	}

	// or with a callback
	vertices.each([](vec3 &v){ v += vec3{1,1,1}; });

	// Array views allow you to work with just a single component as if it
	// was an array of its own.
	array_view_t<float> heights = array_view_create(vertices, &vec3::y);
	for (int32_t i=0; i<heights.count; i+=1) {
		heights[i] = 10;
	}

	// copy_deinterlace will separate out the components into their very own
	// chunk of memory.
	float *arr_heights = heights.copy_deinterlace();
	// Which you will have to free
	free(arr_heights);

	// Array views don't need freed, since they're just a view on some other
	// array.
	heights = {};

	// No deconstructors here, you just have to remember to free it.
	vertices.free();

*/

#pragma once

#include <stdint.h>
#include <stdlib.h>
#include "../sk_memory.h"

//////////////////////////////////////

// Options for customizing code dependencies!

#ifndef ARRAY_MALLOC
#include <malloc.h>
#define ARRAY_MALLOC sk::sk_malloc
#endif
#ifndef ARRAY_FREE
#include <malloc.h>
#define ARRAY_FREE sk::_sk_free
#endif
#ifndef ARRAY_REALLOC
#include <malloc.h>
#define ARRAY_REALLOC sk::sk_realloc
#endif

#ifndef ARRAY_MEMCPY
#include <string.h>
#define ARRAY_MEMCPY memcpy
#endif
#ifndef ARRAY_MEMMOVE
#include <string.h>
#define ARRAY_MEMMOVE memmove
#endif

#ifndef ARRAY_ASSERT
#include <assert.h>
#define ARRAY_ASSERT assert
#endif

//////////////////////////////////////
// array_view_t                     //
//////////////////////////////////////

template <typename T>
struct array_view_t {
	void  *data;
	int32_t count;

	int32_t stride;
	size_t  offset;

	void each(void (*e)(T &))      { for (int32_t i=0; i<count; i++) e((T&)((uint8_t*)data + i*stride + offset)); }
	T   &last()             const  { return (T&)((uint8_t*)data + (count-1)*stride + offset); }
	T   *copy_deinterlace() const;

	inline void set        (int32_t id, const T &val) { *(T*)((uint8_t*)data + id*stride + offset) = val; }
	inline T   &get        (int32_t id) const         { return ((T*)((uint8_t*)data + id*stride + offset))[0]; }
	inline T   &operator[] (int32_t id) const         { return ((T*)((uint8_t*)data + id*stride + offset))[0]; }
};

//////////////////////////////////////
// array_t                          //
//////////////////////////////////////

template <typename T>
struct array_t {
	T      *data;
	int32_t count;
	int32_t capacity;

	int32_t     add        (const T &item)              { if (count+1   > capacity) { resize(capacity * 2 < 4         ? 4         : capacity * 2); } data[count] = item; count += 1; return count - 1; }
	void        add_range  (const T *list, int32_t num) { if (count+num > capacity) { resize(capacity * 2 < count+num ? count+num : capacity * 2); } ARRAY_MEMCPY(&data[count], list, sizeof(T)*num); count += num; }
	void        insert     (int32_t at, const T &item);
	void        resize     (int32_t to_capacity);
	void        trim       ()                         { resize(count); }
	void        remove     (int32_t at);
	void        pop        ()                         { remove(count - 1); }
	void        clear      ()                         { count = 0; }
	T          &last       () const                   { return data[count - 1]; }
	inline void set        (int32_t id, const T &val) { data[id] = val; }
	inline T   &get        (int32_t id) const         { return data[id]; }
	inline T   &operator[] (int32_t id) const         { return data[id]; }
	void        reverse    ();
	array_t<T>  copy       () const;
	void        each       (void (*e)(T &))             { for (int32_t i=0; i<count; i++) e(data[i]); }
	void        each       (void (*e)(const T &)) const { for (int32_t i=0; i<count; i++) e(data[i]); }
	void        each       (void (*e)(void *))          { for (int32_t i=0; i<count; i++) e(data[i]); }
	template <typename U>
	void        each_with  (U *with, void (*e)(U*, const T &)) const { for (int32_t i=0; i<count; i++) e(with, data[i]); }
	template <typename U>
	U           each_with  (void (*e)(U *, const T &))         const { U result = {}; for (int32_t i = 0; i < count; i++) e(&result, data[i]); }
	template <typename U>
	U           each_sum   (U (*e)(const T &))                 const { U result = 0; for (int32_t i = 0; i < count; i++) result += e(data[i]); return result; }
	template <typename U>
	array_t<U>  each_new   (U    (*e)(const T &)) const { array_t<U> result = {}; result.resize(count); for (int32_t i=0; i<count; i++) result.add(e(data[i])); return result; }
	void        free       ();

	static array_t<T> make     (int32_t capacity)                     { array_t<T> result = {}; result.resize(capacity); return result; }
	static array_t<T> make_fill(int32_t capacity, const T &copy_from) { array_t<T> result = {}; result.resize(capacity); result.count = capacity; for(int32_t i=0;i<capacity;i+=1) result.data[i]=copy_from; return result; }
	static array_t<T> make_from(T *use_memory, int32_t count)         { return {use_memory, count, count}; }

	//////////////////////////////////////
	// Linear search methods

	int32_t     index_of   (const T &item) const                                              { for (int32_t i = 0; i < count; i++) if (memcmp(&data[i], &item, sizeof(T)) == 0) return i; return -1; }
	template <typename _T, typename D>
	int32_t     index_where(const D _T::*key, const D &item) const                            { const size_t offset = (size_t)&((_T*)0->*key); for (int32_t i = 0; i < count; i++) if (memcmp(((uint8_t *)&data[i]) + offset, &item, sizeof(D)) == 0) return i; return -1; }
	int32_t     index_where(bool (*c)(const T &item, void *user_data), void *user_data) const { for (int32_t i=0; i<count; i++) if (c(data[i], user_data)) return i; return -1;}
	int32_t     index_where(bool (*c)(const T &item)) const                                   { for (int32_t i=0; i<count; i++) if (c(data[i]))            return i; return -1;}
	int32_t     index_best_small(int32_t (*c)(const T &item)) const                           { int32_t best = 0x7fffffff; int32_t result = -1; for (int32_t i=0; i<count; i++) { int32_t r = c(data[i]); if (r < best) { best = r; result = i;} } return result; }
	int32_t     index_best_large(int32_t (*c)(const T &item)) const                           { int32_t best = 0x80000000; int32_t result = -1; for (int32_t i=0; i<count; i++) { int32_t r = c(data[i]); if (r > best) { best = r; result = i;} } return result;}
	template <typename U>
	int32_t     index_best_small_with(U with, int32_t (*c)(U, const T &item)) const                           { int32_t best = 0x7fffffff; int32_t result = -1; for (int32_t i=0; i<count; i++) { int32_t r = c(with, data[i]); if (r < best) { best = r; result = i;} } return result; }
	template <typename U>
	int32_t     index_best_large_with(U with, int32_t (*c)(U, const T &item)) const                           { int32_t best = -0x80000000; int32_t result = -1; for (int32_t i=0; i<count; i++) { int32_t r = c(with, data[i]); if (r > best) { best = r; result = i;} } return result; }

	//////////////////////////////////////
	// Binary search methods

	int32_t binary_search(const T &item) const;

	// Extra template parameters mean this needs completely defined right here.
	// Does not work on pointers.
	template <typename _T, typename D>
	int32_t binary_search(const D _T::*key, const D &item) const {
		array_view_t<D> view = array_view_t<D>{data, count, sizeof(_T), (size_t)&((_T*)nullptr->*key)};
		int32_t l = 0, r = view.count - 1;
		while (l <= r) {
			int32_t mid     = (l+r) / 2;
			D       mid_val = view[mid];
			if      (mid_val < item) l = mid + 1;
			else if (mid_val > item) r = mid - 1;
			else                       return mid;
		}
		return r < 0 ? r : -(r+2);
	}

	/*template <typename D>
	int32_t binary_search(D (*get_key)(T item), D item) const {
		int32_t l = 0, r = count - 1;
		while (l <= r) {
			int32_t mid = (l + r) / 2;
			D       mid_val = get_key(data[mid]);
			if      (mid_val < item) l = mid + 1;
			else if (mid_val > item) r = mid - 1;
			else                       return mid;
		}
		return r < 0 ? r : -(r + 2);
	}*/

	//////////////////////////////////////
	// Sort methods

	void sort     (int32_t (*compare)(const T&a, const T&b)) { qsort(data, count, sizeof(T), (int (*)(void const*, void const*))compare); }
	void sort     ()                                         { qsort(data, count, sizeof(T), [](const void *a, const void *b) {T fa = *(T*)a, fb = *(T*)b; return (int32_t)((fa > fb) - (fa < fb));}); }
	void sort_desc()                                         { qsort(data, count, sizeof(T), [](const void *a, const void *b) {T fa = *(T*)a, fb = *(T*)b; return (int32_t)((fa < fb) - (fa > fb));}); }

	template <typename _T, typename D, D _T::*key>
	void sort()  {
		qsort((uint8_t*)data, count, sizeof(_T), [](const void *a, const void *b) { 
			size_t offset = (size_t)&((_T*)0->*key); // would love for a way to constexpr this
			D fa = *(D*)((uint8_t*)a + offset), fb = *(D*)((uint8_t*)b + offset); return (int32_t)((fa > fb) - (fa < fb)); 
		}); 
	}

	template <typename _T, typename D, D _T::*key>
	void sort_desc()  {
		qsort((uint8_t*)data, count, sizeof(_T), [](const void *a, const void *b) { 
			size_t offset = (size_t)&((_T*)0->*key); // would love for a way to constexpr this
			D fa = *(D*)((uint8_t*)a + offset), fb = *(D*)((uint8_t*)b + offset); return (int32_t)((fa < fb) - (fa > fb)); 
		});
	}

	void _array_reorder(void** list, int32_t item_size, int32_t count, int32_t* sort_order) {
		uint8_t* src    = (uint8_t*)*list;
		uint8_t* result = (uint8_t*)ARRAY_MALLOC(item_size * capacity);

		for (int32_t i = 0; i < count; i++) {
			memcpy(&result[i * item_size], &src[sort_order[i] * item_size], item_size);
		}

		*list = result;
		ARRAY_FREE(src);
	}
	
	void reorder(int32_t* item_order) {
		_array_reorder((void**)&data, sizeof(T), count, item_order);
	}
};

//////////////////////////////////////
// hashmap_t                        //
//////////////////////////////////////

template <typename K, typename T>
struct hashmap_t {
	array_t<uint64_t> hashes;
	array_t<T>        items;

	uint64_t _hash(const K &key) const {
		uint64_t       hash  = 14695981039346656037UL;
		const uint8_t *bytes = (const uint8_t *)&key;
		for (int32_t i=0; i<sizeof(K); i++)
			hash = (hash ^ bytes[i]) * 1099511628211;
		return hash;
	}

	int32_t add(const K &key, const T &value) {
		uint64_t hash = _hash(key);
		int32_t  id   = hashes.binary_search(hash);
		if (id < 0) {
			id = ~id;
			hashes.insert(id, hash );
			items .insert(id, value);
		}
		return id;
	}
	int32_t add_or_set(const K &key, const T &value) {
		uint64_t hash = _hash(key);
		int32_t  id   = hashes.binary_search(hash);
		if (id < 0) {
			id = ~id;
			hashes.insert(id, hash );
			items .insert(id, value);
		} else {
			hashes[id] = hash;
			items [id] = value;
		}
		return id;
	}

	T       *get      (const K &key)                         const { int32_t id = hashes.binary_search(_hash(key)); return id<0 ? nullptr       : &items[id]; }
	const T &get_or   (const K &key, const T &default_value) const { int32_t id = hashes.binary_search(_hash(key)); return id<0 ? default_value :  items[id]; }
	int32_t  contains (const K &key)                         const { return hashes.binary_search(_hash(key)); }
	void     free     ()                                           { hashes.free(); items.free(); }
	void     remove   (const K& key)                               { int32_t id = hashes.binary_search(_hash(key)); if (id >= 0) { hashes.remove(id); items.remove(id); } }
	void     remove_at(const int32_t at)                           { if (at >= 0) { hashes.remove(at); items.remove(at); } }
};

//////////////////////////////////////
// array_t methods                  //
//////////////////////////////////////

template <typename T>
int32_t array_t<T>::binary_search(const T &item) const {
	int32_t l = 0, r = count - 1;
	while (l <= r) {
		int32_t mid = (l+r) / 2;
		if      (get(mid) < item) l = mid + 1;
		else if (get(mid) > item) r = mid - 1;
		else                      return mid;
	}
	return r < 0 ? r : -(r+2);
}

//////////////////////////////////////

template <typename T>
void array_t<T>::resize(int32_t to_capacity) {
	data     = (T*)ARRAY_REALLOC(data, sizeof(T) * to_capacity);
	capacity = to_capacity;
	if (count > to_capacity)
		count = to_capacity;
}

//////////////////////////////////////

template <typename T>
void array_t<T>::free() {
	ARRAY_FREE(data); 
	*this = {}; 
}

//////////////////////////////////////

template <typename T>
array_t<T> array_t<T>::copy() const { 
	array_t<T> result = { 
		(T*)ARRAY_MALLOC(sizeof(T) * capacity), 
		count, 
		capacity 
	}; 
	ARRAY_MEMCPY(result.data, data, sizeof(T) * count); 
	return result; 
}

//////////////////////////////////////

template <typename T>
void array_t<T>::remove(int32_t aAt) {
	ARRAY_ASSERT(aAt < count);

	ARRAY_MEMMOVE(&data[aAt], &data[aAt+1], (count - (aAt + 1))*sizeof(T));
	count -= 1;
}

//////////////////////////////////////

template <typename T>
void array_t<T>::insert(int32_t aAt, const T &item) {
	ARRAY_ASSERT(aAt <= count);

	if (count+1 > capacity) 
		resize(capacity<1?1:capacity*2);

	ARRAY_MEMMOVE(&data[aAt+1], &data[aAt], (count-aAt)*sizeof(T));
	ARRAY_MEMCPY (&data[aAt], &item, sizeof(T));
	count += 1;
}

//////////////////////////////////////

template <typename T>
void array_t<T>::reverse() {
	for(int32_t i=0; i<count/2; i+=1) {
		T tmp = this->get(i);
		this->set(i, this->get(count - i - 1));
		this->set(count - i - 1, tmp);
	}
}

//////////////////////////////////////
// array_view_t methods             //
//////////////////////////////////////

template <typename D, typename T>
inline static array_view_t<D> array_view_create(const T *data, int32_t count, D T::*key) {
	return array_view_t<D>{data, count, sizeof(T), (int32_t)&((T*)nullptr->*key)};
}

//////////////////////////////////////

// can be called like this: array_view_create(arr_of_vec3, &vec3::x);
template <typename D, typename T>
inline static array_view_t<D> array_view_create(const array_t<T> &src, D T::*key) {
	return array_view_t<D>{src.data, src.count, sizeof(T), (int32_t)&((T*)nullptr->*key)};
}

//////////////////////////////////////

template <typename T>
inline static array_view_t<T> array_view_create(const array_t<T> &src) {
	return array_view_t<T>{src.data, src.count, sizeof(T), 0};
}

//////////////////////////////////////

template <typename T>
T *array_view_t<T>::copy_deinterlace() const {
	T *result = (T*)ARRAY_MALLOC(sizeof(T) * count);

	for (int32_t i=0; i<count; i++) {
		result[i] = this->get(i);
	}
	return result;
}

//////////////////////////////////////

/*
------------------------------------------------------------------------------
This software is available under 2 licenses -- choose whichever you prefer.
------------------------------------------------------------------------------
ALTERNATIVE A - MIT License
Copyright (c) 2020 Nick Klingensmith
Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
of the Software, and to permit persons to whom the Software is furnished to do
so, subject to the following conditions:
The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
------------------------------------------------------------------------------
ALTERNATIVE B - Public Domain (www.unlicense.org)
This is free and unencumbered software released into the public domain.
Anyone is free to copy, modify, publish, use, compile, sell, or distribute this
software, either in source code form or as a compiled binary, for any purpose,
commercial or non-commercial, and by any means.
In jurisdictions that recognize copyright laws, the author or authors of this
software dedicate any and all copyright interest in the software to the public
domain. We make this dedication for the benefit of the public at large and to
the detriment of our heirs and successors. We intend this dedication to be an
overt act of relinquishment in perpetuity of all present and future rights to
this software under copyright law.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
------------------------------------------------------------------------------
*/