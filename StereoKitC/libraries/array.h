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

	Notes: array.h uses size_t instead of int32_t for performance. It 
	eliminates at least one ASM instruction in access and set code.

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
	for (size_t i=0; i<vertices.count; i+=1) {
		vertices[i] += vec3{1,1,1};
	}

	// or with a callback
	vertices.each([](vec3 &v){ v += vec3{1,1,1}; });

	// Array views allow you to work with just a single component as if it
	// was an array of its own.
	array_view_t<float> heights = array_view_create(vertices, &vec3::y);
	for (size_t i=0; i<heights.count; i+=1) {
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

//////////////////////////////////////

// Options for customizing code dependencies!

#ifndef ARRAY_MALLOC
#include <malloc.h>
#define ARRAY_MALLOC malloc
#endif
#ifndef ARRAY_FREE
#include <malloc.h>
#define ARRAY_FREE ::free
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
	size_t count;

	size_t stride;
	size_t offset;

	void each(void (*e)(T &))      { for (size_t i=0; i<count; i++) e((T&)((uint8_t*)data + i*stride + offset)); }
	T   &last()             const  { return (T&)((uint8_t*)data + (count-1)*stride + offset); }
	T   *copy_deinterlace() const;

	inline void set        (size_t id, const T &val) { *(T*)((uint8_t*)data + id*stride + offset) = val; }
	inline T   &get        (size_t id) const         { return ((T*)((uint8_t*)data + id*stride + offset))[0]; }
	inline T   &operator[] (size_t id) const         { return ((T*)((uint8_t*)data + id*stride + offset))[0]; }
};

//////////////////////////////////////
// array_t                          //
//////////////////////////////////////

template <typename T>
struct array_t {
	T     *data;
	size_t count;
	size_t capacity;

	size_t      add        (const T &item)           { if (count+1 > capacity) { resize(capacity * 2 < 4 ? 4 : capacity * 2); } data[count] = item; count += 1; return count - 1; }
	void        insert     (size_t at, const T &item);
	void        resize     (size_t to_capacity);
	void        trim       ()                        { resize(count); }
	void        remove     (size_t at);
	void        pop        ()                        { remove(count - 1); }
	void        clear      ()                        { count = 0; }
	T          &last       () const                  { return data[count - 1]; }
	inline void set        (size_t id, const T &val) { data[id] = val; }
	inline T   &get        (size_t id) const         { return data[id]; }
	inline T   &operator[] (size_t id) const         { return data[id]; }
	void        reverse    ();
	array_t<T>  copy       () const;
	void        each       (void (*e)(T &))          { for (size_t i=0; i<count; i++) e(data[i]); }
	void        free       ();

	//////////////////////////////////////
	// Linear search methods

	int64_t     index_of   (const T &item) const                                              { for (size_t i = 0; i < count; i++) if (memcmp(data[i], item, sizeof(T)) == 0) return i; return -1; }
	template <typename _T, typename D>
	int64_t     index_where(const D _T::*key, const D &item) const                            { const size_t offset = (size_t)&((_T*)0->*key); for (size_t i = 0; i < count; i++) if (memcmp(((uint8_t *)&data[i]) + offset, &item, sizeof(D)) == 0) return i; return -1; }
	int64_t     index_where(bool (*c)(const T &item, void *user_data), void *user_data) const { for (size_t i=0; i<count; i++) if (c(data[i], user_data)) return i; return -1;}
	int64_t     index_where(bool (*c)(const T &item)) const                                   { for (size_t i=0; i<count; i++) if (c(data[i]))            return i; return -1;}

	//////////////////////////////////////
	// Binary search methods

	int64_t binary_search(const T &item) const;

	// Extra template parameters mean this needs completely defined right here
	template <typename _T, typename D>
	int64_t binary_search(const D _T::*key, const D &item) const {
		array_view_t<D> view = array_view_t<D>{data, count, sizeof(_T), (size_t)&((_T*)nullptr->*key)};
		int64_t l = 0, r = view.count - 1;
		while (l <= r) {
			int64_t mid = (l+r) / 2;
			if      (view[mid] < item) l = mid + 1;
			else if (view[mid] > item) r = mid - 1;
			else                       return mid;
		}
		return r < 0 ? r : -(r+2);
	}

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
		for (size_t i=0; i<sizeof(K); i++)
			hash = (hash ^ bytes[i]) * 1099511628211;
		return hash;
	}

	int64_t add(const K &key, const T &value) {
		uint64_t hash = _hash(key);
		int64_t  id   = hashes.binary_search(hash);
		if (id < 0) {
			id = ~id;
			hashes.insert(id, hash );
			items .insert(id, value);
		}
		return id;
	}
	int64_t add_or_set(const K &key, const T &value) {
		uint64_t hash = _hash(key);
		int64_t  id   = hashes.binary_search(hash);
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

	T       *get     (const K &key)                         const { int64_t id = hashes.binary_search(_hash(key)); return id<0 ? nullptr       : &items[id]; }
	const T &get_or  (const K &key, const T &default_value) const { int64_t id = hashes.binary_search(_hash(key)); return id<0 ? default_value :  items[id]; }
	int64_t  contains(const K &key)                         const { return hashes.binary_search(_hash(key)); }
	void     free    ()                                           { hashes.free(); items.free(); }
};

//////////////////////////////////////
// array_t methods                  //
//////////////////////////////////////

template <typename T>
int64_t array_t<T>::binary_search(const T &item) const {
	int64_t l = 0, r = count - 1;
	while (l <= r) {
		int64_t mid = (l+r) / 2;
		if      (get(mid) < item) l = mid + 1;
		else if (get(mid) > item) r = mid - 1;
		else                      return mid;
	}
	return r < 0 ? r : -(r+2);
}

//////////////////////////////////////

template <typename T>
void array_t<T>::resize(size_t to_capacity) {
	if (count > to_capacity) 
		count = to_capacity;

	void  *old_memory = data;
	void  *new_memory = ARRAY_MALLOC(sizeof(T) * to_capacity); 
	memcpy(new_memory, old_memory, sizeof(T) * count);

	data = (T*)new_memory;
	ARRAY_FREE(old_memory);

	capacity = to_capacity;
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
		ARRAY_MALLOC(sizeof(T) * capacity), 
		count, 
		capacity 
	}; 
	ARRAY_MEMCPY(result.data, data, sizeof(T) * count); 
	return result; 
}

//////////////////////////////////////

template <typename T>
void array_t<T>::remove(size_t aAt) {
	ARRAY_ASSERT(aAt < count);

	ARRAY_MEMMOVE(&data[aAt], &data[aAt+1], (count - (aAt + 1))*sizeof(T));
	count -= 1;
}

//////////////////////////////////////

template <typename T>
void array_t<T>::insert(size_t aAt, const T &item) {
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
	for(size_t i=0; i<count/2; i+=1) {
		T tmp = this->get(i);
		this->set(i, this->get(count - i - 1));
		this->set(count - i - 1, tmp);
	}
}

//////////////////////////////////////
// array_view_t methods             //
//////////////////////////////////////

template <typename D, typename T>
inline static array_view_t<D> array_view_create(const T *data, size_t count, D T::*key) {
	return array_view_t<D>{data, count, sizeof(T), (size_t)&((T*)nullptr->*key)};
}

//////////////////////////////////////

// can be called like this: array_view_create(arr_of_vec3, &vec3::x);
template <typename D, typename T>
inline static array_view_t<D> array_view_create(const array_t<T> &src, D T::*key) {
	return array_view_t<D>{src.data, src.count, sizeof(T), (size_t)&((T*)nullptr->*key)};
}

//////////////////////////////////////

template <typename T>
T *array_view_t<T>::copy_deinterlace() const {
	T *result = (T*)ARRAY_MALLOC(sizeof(T) * count);

	for (size_t i=0; i<count; i++) {
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