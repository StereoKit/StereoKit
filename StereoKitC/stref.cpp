#include "stref.h"

#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <stdarg.h>

char *string_copy(const char *aString) {
	size_t size   = strlen(aString) + 1;
	char  *result = (char*)malloc(size);
	memcpy(result, aString, size);
	return result;
}
char *string_make(stref_t &ref) {
	char *result = string_copy(stref_withend(ref));
	stref_remend(ref);
	return result;
}
char *string_append(char *aBase, uint32_t aCount, ...) {
	size_t baseSize = aBase == nullptr? 0 : strlen(aBase);

	size_t  addSize = 0;
	va_list args;
	va_start(args, aCount);
	for (uint32_t i = 0; i < aCount; i++) {
		addSize += strlen(va_arg(args, char *));
	}
	va_end(args);

	char *result = (char*)realloc(aBase, baseSize + addSize + 1);
	char *curr   = result + baseSize;
	va_start(args, aCount);
	for (uint32_t i = 0; i < aCount; i++) {
		char *arg = va_arg(args, char *);
		size_t len = strlen(arg);
		memcpy(curr, arg, len + 1);
		curr += len;
	}
	va_end(args);

	return result;
}
bool  string_eq(const char *a, const char *b) {
	while (*a != '\0' && *b != '\0') {
		if (*a != *b)
			return false;
		a++;
		b++;
	}
	return *a == *b;
}
bool  string_endswith(const char *a, const char *end) {
	size_t len_a   = strlen(a);
	size_t len_end = strlen(end);
	if (len_end > len_a)
		return false;
	a = a + (len_a - len_end);

	while (*a != '\0' && *end != '\0') {
		if (*a != *end)
			return false;
		a++;
		end++;
	}
	return *a == *end;
}

bool  stref_equals(stref_t &aRef, const char *aIs) {
	assert(aRef.temp == false);

	const char *currRef = aRef.start;
	uint32_t curr = 0;
	while (*currRef != '\0' && *aIs != '\0' && curr < aRef.length) {
		if (*currRef != *aIs)
			return false;
		currRef++;
		aIs++;
		curr++;
	}
	return curr == aRef.length;
}
bool  stref_equals(stref_t &a, stref_t &b) {
	assert(a.temp == false);
	assert(b.temp == false);
	if (a.length != b.length)
		return false;

	const char *a_curr = a.start;
	const char *b_curr = b.start;
	uint32_t curr = 0;
	while (*a_curr != '\0' && *b_curr != '\0' && curr < a.length) {
		if (*a_curr != *b_curr)
			return false;
		a_curr++;
		b_curr++;
		curr++;
	}
	return true;
}
int32_t   stref_indexof(stref_t &aRef, char aChar) {
	for (uint32_t i = 0; i < aRef.length; i++) {
		if (aRef.start[i] == aChar)
			return i;
	}
	return -1;
}
int32_t   stref_lastof(stref_t &aRef, char aChar) {
	int32_t result = -1;
	for (uint32_t i = 0; i < aRef.length; i++) {
		if (aRef.start[i] == aChar)
			result = i;
	}
	return result;
}

char *stref_copy(stref_t &aRef) {
	assert(aRef.temp == false);

	char  *result = (char*)malloc((uint64_t)aRef.length+1);
	memcpy(result, aRef.start, aRef.length);
	result[aRef.length] = '\0';
	return result;
}

const char *stref_withend(stref_t &aRef) {
	assert(aRef.temp == false);

	aRef.temp_end = aRef.start[aRef.length];
	aRef.temp = true;
	((char*)aRef.start)[aRef.length] = '\0';

	return aRef.start;
}
void  stref_remend(stref_t &aRef) {
	assert(aRef.temp == true);

	((char*)aRef.start)[aRef.length] = aRef.temp_end;
	aRef.temp_end = 0;
	aRef.temp = false;
}

stref_t  stref_make(const char *aSource) {
	stref_t result;
	result.start    = aSource;
	result.length   = strlen(aSource);
	result.temp_end = 0;
	result.temp     = false;
	return result;
}

stref_t stref_substr(stref_t &aRef, uint32_t aStart, uint32_t aLength) {
	assert(aRef.temp == false);

	stref_t result;
	result.start    = aRef.start + aStart;
	result.length   = aLength;
	result.temp_end = 0;
	result.temp     = false;
	return result;
}

stref_t stref_substr(const char *aSource, uint32_t aLength) {
	stref_t result;
	result.start    = aSource;
	result.length   = aLength;
	result.temp_end = 0;
	result.temp     = false;
	return result;
}

bool is_white(char c) {
	return c == ' ' || c == '\t' || c=='\n' || c=='\r';
}
void  stref_trim(stref_t &aRef) {
	while (aRef.length > 0 && is_white(aRef.start[0])) {
		aRef.start += 1;
		aRef.length -= 1;
	}
	while (aRef.length > 0 && is_white(aRef.start[aRef.length-1])) {
		aRef.length -= 1;
	}
}
uint32_t  stref_count(stref_t &aRef, char aChar) {
	uint32_t result = 0;
	for (uint32_t i = 0; i < aRef.length; i++) {
		if (aRef.start[i] == aChar)
			result += 1;
	}
	return result;
}
bool stref_nextline(stref_t &from, stref_t &curr_line) {
	assert(curr_line.temp == false);

	if (curr_line.start == nullptr)
		curr_line = stref_substr(from, 0, from.length);

	char *curr = (char*)(curr_line.start);
	while (*curr != '\n' && *curr != '\r' && *curr != '\0') curr++;
	while (*curr == '\n' || *curr == '\r') curr++;
	if (*curr == '\0') return false;

	curr_line.start = curr;
	while (*curr != '\n' && *curr != '\r' && *curr != '\0') curr++;
	curr_line.length = curr - curr_line.start;
	return true;
}
bool stref_nextword(stref_t &line, stref_t &word) {
	assert(word.temp == false);

	if (word.start == nullptr)
		word = stref_substr(line, 0, 0);

	char *curr = (char*)(word.start + word.length);
	while (*curr == ' ') curr++;
	if ( *curr == '\n' || *curr == '\r' || *curr == '\0') return false;

	word.start = curr;
	while (*curr != ' ' && *curr != '\n' && *curr != '\r' && *curr != '\0') curr++;
	word.length = curr - word.start;
	return true;
}
void stref_file_path(const stref_t &filename, stref_t &out_path, stref_t &out_name) {
	out_path = filename;
	int32_t  end    = out_path.length;
	int32_t  slash1 = stref_lastof(out_path, '\\');
	int32_t  slash2 = stref_lastof(out_path, '/');
	uint32_t start  = (slash1 > slash2 ? slash1 : slash2) + 1;
	out_name = stref_substr(out_path, start, end-start);

	if (stref_indexof(out_name, '.') != -1) {
		// Has a '.', must be a filename!
		out_path.length = start-1;
	}
}

// djb2 hash: http://www.cse.yorku.ca/~oz/hash.html
uint64_t string_hash(const char *string) {
	unsigned long hash = 5381;
	int c;
	while (c = *string++)
		hash = ((hash << 5) + hash) + c; // hash * 33 + c
	return hash;
}
uint64_t stref_hash(const stref_t &ref) {
	unsigned long hash = 5381;
	int c;
	for (uint32_t i=0; i<ref.length; i++) {
		c = *(ref.start + i);
		hash = ((hash << 5) + hash) + c; // hash * 33 + c
	}
	return hash;
}