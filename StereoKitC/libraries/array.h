#pragma once

#include <stdint.h>
#include <malloc.h>

template <typename T>
struct array_t {
	T      *data;
	int32_t count;
	int32_t capacity;

	int32_t    add(const T &item)        { if (count+1 >= capacity) { resize(capacity * 2 < 4 ? 4 : capacity * 2); } data[count] = item; count += 1; return count - 1; }
	void       clear()                   { count = 0; }
	void       each(void (*e)(T &))      { for (int32_t i=0; i<count; i++) e(data[i]); }
	T         &last() const              { return data[count - 1]; }
	void       pop()                     { remove(count - 1); }
	void       resize(int32_t to_capacity);
	void       free();
	array_t<T> copy() const;
	void       remove(int32_t at);
	void       insert(int32_t at, const T &item);
	void       reverse();

	inline void set        (int32_t id, T &val) { data[id] = val; }
	inline T   &get        (int32_t id) const   { return data[id]; }
	inline T   &operator[] (int32_t id) const   { return data[id]; }
};

#include <assert.h>
#include <string.h>

//////////////////////////////////////

template <typename T>
void array_t<T>::resize(int32_t to_capacity) {
	if (count > to_capacity) 
		count = to_capacity;

	void  *old_memory = data;
	void  *new_memory = malloc(sizeof(T) * to_capacity); 
	memcpy(new_memory, old_memory, sizeof(T) * count);

	data = (T*)new_memory;
	::free(old_memory);

	capacity = to_capacity;
}

//////////////////////////////////////

template <typename T>
void array_t<T>::free() {
	::free(data); 
	*this = {}; 
}

//////////////////////////////////////

template <typename T>
array_t<T> array_t<T>::copy() const { 
	array_t<T> result = { 
		malloc(sizeof(T) * capacity), 
		count, 
		capacity 
	}; 
	memcpy(result.data, data, sizeof(T) * count); 
	return result; 
}

//////////////////////////////////////

template <typename T>
void array_t<T>::remove(int32_t aAt) {
	assert(aAt < count);

	int64_t  at  = aAt;
	uint8_t *arr = (uint8_t*)data;
	memmove(arr + at * sizeof(T), arr + (at + 1)*sizeof(T), (arr + count*sizeof(T)) - (arr + (at + 1)*sizeof(T)));
	count -= 1;
}

//////////////////////////////////////

template <typename T>
void array_t<T>::insert(int32_t aAt, const T &item) {
	assert(aAt <= count);

	if (count+1 > capacity) 
		resize(capacity<1?1:capacity*2);

	int64_t  at  = aAt;
	uint8_t *arr = (uint8_t*)data;
	memmove(arr + (at + 1)*sizeof(T), arr + at*sizeof(T), (arr + count*sizeof(T)) - (arr + at*sizeof(T)));
	memcpy (arr + at * sizeof(T), &item, sizeof(T));
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