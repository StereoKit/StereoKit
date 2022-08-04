#include "stref.h"

#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <stdarg.h>
#include <ctype.h>

#include "../sk_memory.h"
#define STR_MALLOC sk::sk_malloc
#define STR_REALLOC sk::sk_realloc

///////////////////////////////////////////

char *string_copy(const char *aString) {
	size_t size   = strlen(aString) + 1;
	char  *result = (char*)STR_MALLOC(size);
	memcpy(result, aString, size);
	return result;
}

///////////////////////////////////////////

char *string_make(stref_t &ref) {
	char *result = string_copy(stref_withend(ref));
	stref_remend(ref);
	return result;
}

///////////////////////////////////////////

char *string_append(char *aBase, uint32_t aCount, ...) {
	size_t baseSize = aBase == nullptr? 0 : strlen(aBase);

	size_t  addSize = 0;
	va_list args;
	va_start(args, aCount);
	for (uint32_t i = 0; i < aCount; i++) {
		addSize += strlen(va_arg(args, char *));
	}
	va_end(args);

	char *result = (char*)STR_REALLOC(aBase, baseSize + addSize + 1);
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

///////////////////////////////////////////

char *string_substr(const char *a, uint32_t start, uint32_t length) {
	if (length == 0) return nullptr;

	char *result = (char *)STR_MALLOC(length + 1);
	if (result != nullptr) {
		memcpy(result, a + start, length);
		result[length] = '\0';
	}
	return result;
}

///////////////////////////////////////////

bool  string_eq(const char *a, const char *b) {
	if (a == nullptr || b == nullptr) return false;
	while (*a != '\0' && *b != '\0') {
		if (*a != *b)
			return false;
		a++;
		b++;
	}
	return *a == *b;
}

///////////////////////////////////////////

bool  string_eq_nocase(const char *a, const char *b) {
	while (*a != '\0' && *b != '\0') {
		if (tolower(*a) != tolower(*b))
			return false;
		a++;
		b++;
	}
	return *a == *b;
}

///////////////////////////////////////////

bool  string_endswith(const char *a, const char *end, bool case_sensitive) {
	size_t len_a   = strlen(a);
	size_t len_end = strlen(end);
	if (len_end > len_a)
		return false;
	a = a + (len_a - len_end);

	if (!case_sensitive) {
		while (*a != '\0' && *end != '\0') {
			if (tolower(*a) != tolower(*end))
				return false;
			a++;
			end++;
		}
		return tolower(*a) == tolower(*end);
	} else {
		while (*a != '\0' && *end != '\0') {
			if (*a != *end)
				return false;
			a++;
			end++;
		}
		return *a == *end;
	}
}

///////////////////////////////////////////

bool string_startswith(const char *a, const char *is) {
	while (*is != '\0') {
		if (*a == '\0' || *is != *a)
			return false;
		a++;
		is++;
	}
	return true;
}

///////////////////////////////////////////

bool  stref_equals(const stref_t &aRef, const char *aIs) {
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
	return curr == aRef.length && *aIs == '\0';
}

///////////////////////////////////////////

bool  stref_equals(const stref_t &a, const stref_t &b) {
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

///////////////////////////////////////////

bool  stref_startswith(const stref_t &a, const char *is) {
	uint32_t curr = 0;
	while (*is != '\0') {
		if (curr >= a.length || *is != a.start[curr])
			return false;
		curr++;
		is++;
	}
	return true;
}

///////////////////////////////////////////

int32_t   stref_indexof(const stref_t &aRef, char aChar) {
	for (uint32_t i = 0; i < aRef.length; i++) {
		if (aRef.start[i] == aChar)
			return i;
	}
	return -1;
}

///////////////////////////////////////////

int32_t   stref_lastof(const stref_t &aRef, char aChar) {
	int32_t result = -1;
	for (uint32_t i = 0; i < aRef.length; i++) {
		if (aRef.start[i] == aChar)
			result = i;
	}
	return result;
}

///////////////////////////////////////////

char *stref_copy(const stref_t &aRef) {
	assert(aRef.temp == false);

	char  *result = (char*)STR_MALLOC((uint64_t)aRef.length+1);
	memcpy(result, aRef.start, aRef.length);
	result[aRef.length] = '\0';
	return result;
}

///////////////////////////////////////////

void stref_copy_to(const stref_t &ref, char *text, size_t text_size) {
	assert(ref.temp == false);
	size_t length = text_size < ref.length ? text_size : ref.length;
	memcpy(text, ref.start, length);
	text[length] = '\0';
}

///////////////////////////////////////////

const char *stref_withend(stref_t &aRef) {
	assert(aRef.temp == false);

	aRef.temp_end = aRef.start[aRef.length];
	aRef.temp = true;
	((char*)aRef.start)[aRef.length] = '\0';

	return aRef.start;
}

///////////////////////////////////////////

void  stref_remend(stref_t &aRef) {
	assert(aRef.temp == true);

	((char*)aRef.start)[aRef.length] = aRef.temp_end;
	aRef.temp_end = 0;
	aRef.temp = false;
}

///////////////////////////////////////////

stref_t  stref_make(const char *aSource) {
	stref_t result;
	result.start    = aSource;
	result.length   = (uint32_t)strlen(aSource);
	result.temp_end = 0;
	result.temp     = false;
	return result;
}

///////////////////////////////////////////

stref_t stref_substr(const stref_t &aRef, uint32_t aStart, uint32_t aLength) {
	assert(aRef.temp == false);

	stref_t result;
	result.start    = aRef.start + aStart;
	result.length   = aLength;
	result.temp_end = 0;
	result.temp     = false;
	return result;
}

///////////////////////////////////////////

stref_t stref_substr(const char *aSource, uint32_t aLength) {
	stref_t result;
	result.start    = aSource;
	result.length   = aLength;
	result.temp_end = 0;
	result.temp     = false;
	return result;
}

///////////////////////////////////////////

bool is_white(char c) {
	return c == ' ' || c == '\t' || c=='\n' || c=='\r';
}

///////////////////////////////////////////

void  stref_trim(stref_t &aRef) {
	while (aRef.length > 0 && is_white(aRef.start[0])) {
		aRef.start += 1;
		aRef.length -= 1;
	}
	while (aRef.length > 0 && is_white(aRef.start[aRef.length-1])) {
		aRef.length -= 1;
	}
}

///////////////////////////////////////////

uint32_t  stref_count(stref_t &aRef, char aChar) {
	uint32_t result = 0;
	for (uint32_t i = 0; i < aRef.length; i++) {
		if (aRef.start[i] == aChar)
			result += 1;
	}
	return result;
}

///////////////////////////////////////////

bool stref_nextline(stref_t &from, stref_t &curr_line) {
	assert(curr_line.temp == false);

	if (curr_line.start == nullptr) {
		curr_line = stref_substr(from, 0, from.length);
		char *curr = (char*)(curr_line.start);
		while (*curr != '\n' && *curr != '\r' && *curr != '\0') curr++;
		curr_line.length = (uint32_t)(curr - curr_line.start);
		return true;
	}

	char *curr = (char*)(curr_line.start);
	while (*curr != '\n' && *curr != '\r' && *curr != '\0') curr++;
	while (*curr == '\n' || *curr == '\r') curr++;
	if (*curr == '\0') return false;

	curr_line.start = curr;
	while (*curr != '\n' && *curr != '\r' && *curr != '\0') curr++;
	curr_line.length = (uint32_t)(curr - curr_line.start);
	return true;
}

///////////////////////////////////////////

bool stref_nextword(stref_t &line, stref_t &word, char separator, char capture_char_start, char capture_char_end, bool *out_capture_error) {
	assert(word.temp == false);
	if (out_capture_error != nullptr)
		*out_capture_error = false;
	if (word.start == nullptr)
		word = stref_substr(line, 0, 0);

	char *curr = (char*)(word.start + word.length);
	if (*curr == separator) curr++;
	while (*curr == ' ' || *curr == '\t') {
		curr++; 
	}
	if ( *curr == '\n' || *curr == '\r' || *curr == '\0') return false;

	word.start = curr;

	// Check if we're looking at a capture group
	int capture_count = 0;
	bool want_capture = capture_char_start != '\0';
	if (want_capture && *curr == capture_char_start) {
		capture_count = 1;
		curr++;
	}
	
	while (
		(*curr != separator || capture_count > 0) &&                            // While we're still part of the same word or capture group
		!(want_capture && capture_count == 0 && *curr == capture_char_start) && // Check if we're running into an adjacent capture group
		(*curr != '\n' && *curr != '\r' && *curr != '\0') &&                    // While we're still on the same line
		(curr < line.start+line.length))                                        // And make sure it's within the confines of the stref we're provided
	{
		if (want_capture) {
			if (*curr == capture_char_start)
				capture_count += 1;
			if (*curr == capture_char_end) {
				capture_count -= 1;
				if (capture_count == 0) {
					curr++;
					break;
				}
			}
		}
		curr++;
	}
	word.length = (uint32_t)(curr - word.start);
	if (out_capture_error != nullptr)
		*out_capture_error = capture_count > 0;
	return true;
}

///////////////////////////////////////////

void stref_file_path(const stref_t &filename, stref_t &out_path, stref_t &out_name) {
	out_path = stref_t{ filename.start, 0 };
	int32_t  end    = filename.length;
	int32_t  slash1 = stref_lastof(filename, '\\');
	int32_t  slash2 = stref_lastof(filename, '/');
	uint32_t start  = (slash1 > slash2 ? slash1 : slash2) + 1;
	out_name = stref_substr(filename, start, end-start);

	if (stref_indexof(out_name, '.') != -1) {
		// Has a '.', must be a filename!
		out_path.length = (int)start-1 < 0 ? 0 : start-1;
	}
}

///////////////////////////////////////////

stref_t stref_stripcapture(stref_t &word, char capture_char_start, char capture_char_end) {
	if (word.length < 2) return word;
	stref_t result = word;
	if (*result.start == capture_char_start) {
		result.start++;
		result.length--;
	}
	if (*(result.start+result.length-1) == capture_char_end) {
		result.length--;
	}
	stref_trim(result);
	return result;
}

///////////////////////////////////////////

float stref_to_f(const stref_t &ref) {
	char text[32];
	stref_copy_to(ref, text, 32);
	return (float)atof(text);
}

///////////////////////////////////////////

int32_t  stref_to_i(const stref_t &ref) {
	char text[32];
	stref_copy_to(ref, text, 32);
	return atoi(text);
}