#pragma once

#include <stdint.h>
#include <string.h>

///////////////////////////////////////////

// Source:
// https://github.com/skeeto/branchless-utf8
// Should switch this to a SIMD implementation later, possibly:
// https://dirtyhandscoding.github.io/posts/utf8lut-vectorized-utf-8-converter-decoding-utf-8.html
inline char32_t utf8_decode_fast(const char *utf8_str, const char **out_next_char) {
	static const uint32_t lengths[] = {
		1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
		0, 0, 0, 0, 0, 0, 0, 0, 2, 2, 2, 2, 3, 3, 4, 0
	};
	static const int32_t  masks[]  = {0x00, 0x7f, 0x1f, 0x0f, 0x07};
	static const uint32_t mins[]   = {4194304, 0, 128, 2048, 65536};
	static const int32_t  shiftc[] = {0, 18, 12, 6, 0};
	static const int32_t  shifte[] = {0, 6, 4, 2, 0};

	const unsigned char *s   = (unsigned char *)utf8_str;
	int32_t              len = lengths[s[0] >> 3];

	// Compute the pointer to the next character early so that the next
	// iteration can start working on the next character. Neither Clang
	// nor GCC figure out this reordering on their own.
	const unsigned char *next = s + len + !len;

	// Assume a four-byte character and load four bytes. Unused bits are
	// shifted out.
	char32_t result = (char32_t)(s[0] & masks[len]) << 18;
	result |= (char32_t)(s[1] & 0x3f) << 12;
	result |= (char32_t)(s[2] & 0x3f) <<  6;
	result |= (char32_t)(s[3] & 0x3f) <<  0;
	result >>= shiftc[len];

	*out_next_char = (const char *)next;
	return result;
}

///////////////////////////////////////////

inline bool utf8_decode_fast_b(const char *utf8_str, const char **out_next_char, char32_t *out_char) {
	if (*utf8_str == 0) { *out_char = 0; return false; }
	*out_char = utf8_decode_fast(utf8_str, out_next_char);
	return *out_char != 0;
}

///////////////////////////////////////////

inline char32_t utf8_decode(const char *utf8_str, const char **out_next_char, int32_t *out_err) {
	static const char lengths[] = {
		1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
		0, 0, 0, 0, 0, 0, 0, 0, 2, 2, 2, 2, 3, 3, 4, 0
	};
	static const int32_t  masks[]  = {0x00, 0x7f, 0x1f, 0x0f, 0x07};
	static const uint32_t mins[]   = {4194304, 0, 128, 2048, 65536};
	static const int32_t  shiftc[] = {0, 18, 12, 6, 0};
	static const int32_t  shifte[] = {0, 6, 4, 2, 0};

	const char *s   = utf8_str;
	int32_t     len = lengths[s[0] >> 3];

	// Compute the pointer to the next character early so that the next
	// iteration can start working on the next character. Neither Clang
	// nor GCC figure out this reordering on their own.
	const char *next = s + len + !len;

	// Assume a four-byte character and load four bytes. Unused bits are
	// shifted out.
	char32_t result = (char32_t)(s[0] & masks[len]) << 18;
	result |= (char32_t)(s[1] & 0x3f) << 12;
	result |= (char32_t)(s[2] & 0x3f) <<  6;
	result |= (char32_t)(s[3] & 0x3f) <<  0;
	result >>= shiftc[len];

	// Accumulate the various error conditions.
	*out_err  = (result < mins[len]) << 6; // non-canonical encoding
	*out_err |= ((result >> 11) == 0x1b) << 7;  // surrogate half?
	*out_err |= (result > 0x10FFFF) << 8;  // out of range?
	*out_err |= (s[1] & 0xc0) >> 2;
	*out_err |= (s[2] & 0xc0) >> 4;
	*out_err |= (s[3]       ) >> 6;
	*out_err ^= 0x2a; // top two bits of each tail byte correct?
	*out_err >>= shifte[len];

	*out_next_char = next;
	return result;
}

///////////////////////////////////////////

inline int32_t utf8_encode_units(char32_t ch) {
	if      (ch <= 0x7F)     return 1;
	else if (ch <= 0x07FF)   return 2;
	else if (ch <= 0xFFFF)   return 3;
	else if (ch <= 0x10FFFF) return 4;
	else                     return 0;
}

///////////////////////////////////////////

inline int32_t utf16_encode_units(char32_t ch) {
	if (ch < 0x00010000L) return 1;
	if (ch < 0x00110000L) return 2;
	return 0;
}

///////////////////////////////////////////

inline int32_t utf8_encode(char *out, char32_t utf) {
	if (utf <= 0x7F) {
		// Plain ASCII
		out[0] = (char) utf;
		return 1;
	} else if (utf <= 0x07FF) {
		// 2-byte unicode
		out[0] = (char) (((utf >> 6) & 0x1F) | 0xC0);
		out[1] = (char) (((utf >> 0) & 0x3F) | 0x80);
		return 2;
	} else if (utf <= 0xFFFF) {
		// 3-byte unicode
		out[0] = (char) (((utf >> 12) & 0x0F) | 0xE0);
		out[1] = (char) (((utf >>  6) & 0x3F) | 0x80);
		out[2] = (char) (((utf >>  0) & 0x3F) | 0x80);
		return 3;
	} else if (utf <= 0x10FFFF) {
		// 4-byte unicode
		out[0] = (char) (((utf >> 18) & 0x07) | 0xF0);
		out[1] = (char) (((utf >> 12) & 0x3F) | 0x80);
		out[2] = (char) (((utf >>  6) & 0x3F) | 0x80);
		out[3] = (char) (((utf >>  0) & 0x3F) | 0x80);
		return 4;
	}
	return 0;
}

///////////////////////////////////////////

inline int32_t utf8_encode_append(char *buffer, size_t size, char32_t ch) {
	int32_t count = utf8_encode_units(ch);
	size_t  len   = strlen(buffer);
	if (len + count + 1 >= size) {
		return 0;
	} else {
		count = utf8_encode(&buffer[len], ch);
		buffer[len + count] = '\0';
		return count;
	}
}

///////////////////////////////////////////

inline bool utf8_is_start(char ch) {
	return 
		(ch & 0x80) == 0 ||
		(ch & 0xC0) == 0xC0;
}

///////////////////////////////////////////

inline size_t strlen(const char16_t *str) {
	if(!str) return 0;
	const char16_t* curr = str;
	while (*curr) curr++;
	return curr-str;
}

///////////////////////////////////////////

inline size_t utf_charlen(const char* str_utf8) {
	if (!str_utf8) return 0;
	
	// Single byte UTF8 characters all start with 0b0, so anything smaller than
	// 0b1 must be single byte!
	const char single_byte = 0b10000000;
	// The first byte of multibyte UTF8 characters all begin with 0b11, so
	// anything larger than 0b10111111 is a multibye character indicator!
	const char multi_byte = 0b10111111;
	
	size_t      result = 0;
	const char* curr   = str_utf8;
	while (*curr != '\0') {
		if (*curr < single_byte || *curr > multi_byte) { result += 1; }
		curr++;
	}
	return result;
}

///////////////////////////////////////////

inline size_t utf_charlen(const char16_t* str_utf16) {
	if (!str_utf16) return 0;
	
	size_t          result = 0;
	const char16_t* curr   = str_utf16;
	while (*curr != '\0') {
		// Pairs start with a value in the 0xD800–0xDBFF range
		// Singles have a value in the 0x0000–0xD7FF, 0xE000–0xFFFF ranges
		if (*curr < 0xDC00 || *curr > 0xDFFF) { result += 1; }
		curr++;
	}
	return result;
}

///////////////////////////////////////////

inline const char* utf_at_char(const char* str_utf8, int32_t index) {
	if (!str_utf8) return nullptr;

	// Single byte UTF8 characters all start with 0b0, so anything smaller than
	// 0b1 must be single byte!
	const char single_byte = 0b10000000;
	// The first byte of multibyte UTF8 characters all begin with 0b11, so
	// anything larger than 0b10111111 is a multibye character indicator!
	const char multi_byte = 0b10111111;

	size_t      result = 0;
	const char* curr   = str_utf8;
	while (*curr != '\0') {
		if (result == index) return curr;
		if (*curr < single_byte || *curr > multi_byte) { result += 1; }
		curr++;
	}
	return nullptr;
}

///////////////////////////////////////////

inline const char16_t* utf_at_char(const char16_t* str_utf16, int32_t index) {
	if (!str_utf16) return nullptr;

	size_t          result = 0;
	const char16_t* curr = str_utf16;
	while (*curr != '\0') {
		if (result == index) return curr;
		// Pairs start with a value in the 0xD800–0xDBFF range
		// Singles have a value in the 0x0000–0xD7FF, 0xE000–0xFFFF ranges
		if (*curr < 0xDC00 || *curr > 0xDFFF) { result += 1; }
		curr++;
	}
	return nullptr;
}

///////////////////////////////////////////

inline bool utf_remove_chars(char* at_utf8, int32_t count) {
	if (!at_utf8) return false;

	size_t remove = 0;
	char*  curr   = at_utf8;
	for (size_t i = 0; i < count; i++)
	{
		if      (*curr == '\0')                      break;
		else if ((*curr & 0b10000000) == 0         ) remove += 1;
		else if ((*curr & 0b11100000) == 0b11000000) remove += 2;
		else if ((*curr & 0b11110000) == 0b11100000) remove += 3;
		else if ((*curr & 0b11111000) == 0b11110000) remove += 4;
		curr = at_utf8 + remove;
	}

	size_t  len = strlen(at_utf8);
	int32_t end = len - remove;
	for (size_t i = 0; i < end; i++) 
		at_utf8[i] = at_utf8[i + remove];
	at_utf8[end] = '\0';
}

///////////////////////////////////////////

inline bool utf_insert_char(const char* buffer_utf8_start, size_t buffer_length, char* at_utf8, char32_t character) {
	int32_t units = utf8_encode_units(character);

	// Ensure there's enough memory in the string
	if (buffer_length - (at_utf8 - buffer_utf8_start) < units) return false;

	// Shift the string to make room for the new character
	size_t len = strlen(at_utf8);
	for (int32_t i = len + units; i >= units; i--)
		at_utf8[i] = at_utf8[i - units];

	// And insert the new character
	utf8_encode(at_utf8, character);
	return true;
}

///////////////////////////////////////////

inline bool utf_remove_chars(char16_t* at_utf16, int32_t count) {
	if (!at_utf16) return false;

	size_t    remove = 0;
	char16_t* curr   = at_utf16;
	for (size_t i = 0; i < count; i++)
	{
		if      (*curr == '\0')                            break;
		else if (*at_utf16 < 0xD800 || *at_utf16 > 0xDFFF) remove += 1;
		else                                               remove += 2;
		curr = at_utf16 + remove;
	}
	
	size_t  len    = strlen(at_utf16);
	int32_t end    = len - remove;
	for (size_t i = 0; i < end; i++) 
		at_utf16[i] = at_utf16[i + remove];
	at_utf16[end] = '\0';
}

///////////////////////////////////////////

inline char32_t utf16_decode_fast(const char16_t *utf16_str, const char16_t **out_next) {
	if (utf16_str[0] < 0xd800 || utf16_str[0] > 0xdfff) {
		char32_t result = utf16_str[0];
		*out_next = utf16_str + 1;
		return result;

	} else if (utf16_str[0] >= 0xd800 && utf16_str[0] <= 0xdbff &&
	           utf16_str[1] >= 0xdc00 && utf16_str[1] <= 0xdfff) {

		char32_t result = (((utf16_str[0] & 0x03ffL) << 10) | ((utf16_str[1] & 0x03ffL) <<  0)) + 0x00010000L;
		*out_next = utf16_str + 2;
		return result;
	}
	return 0;
}

///////////////////////////////////////////

inline bool utf16_decode_fast_b(const char16_t *utf16_str, const char16_t **out_next_char, char32_t *out_char) {
	if (*utf16_str == 0) { *out_char = 0; return false; }
	*out_char = utf16_decode_fast(utf16_str, out_next_char);
	return *out_char != 0;
}

///////////////////////////////////////////

inline int32_t utf16_encode(char16_t *out, char32_t utf) {
	if (utf < 0x00010000L) {
		out[0] = (char16_t)utf;
		return 1;
	} else if (utf < 0x00110000L) {
		utf -= 0x00010000L;

		out[0] = ((utf >> 10) & 0x3ff) | 0xd800;
		out[1] = ((utf >> 0 ) & 0x3ff) | 0xdc00;
		return 2;
	}
	return 0;
}

///////////////////////////////////////////

inline bool utf_insert_char(const char16_t* buffer_utf16_start, size_t buffer_length, char16_t* at_utf16, char32_t character) {
	int32_t units = utf16_encode_units(character);

	// Ensure there's enough memory in the string
	if (buffer_length - (at_utf16 - buffer_utf16_start) < units) return false;

	// Shift the string to make room for the new character
	size_t len = strlen(at_utf16);
	for (int32_t i = len+units; i >= units; i--)
		at_utf16[i] = at_utf16[i - units];

	// And insert the new character
	utf16_encode(at_utf16, character);
	return true;
}

///////////////////////////////////////////

inline int32_t utf16_encode_append(char16_t *buffer, size_t size, char32_t ch) {
	int32_t count = utf16_encode_units(ch);
	size_t  len   = strlen(buffer);
	if (len + count + 1 >= size) {
		return 0;
	} else {
		count = utf16_encode(&buffer[len], ch);
		buffer[len + count] = '\0';
		return count;
	}
}

///////////////////////////////////////////

inline bool utf16_is_start(char16_t ch) {
	return !(ch >= 0xdc00 && ch <= 0xdfff);
}