/*Licensed under MIT or Public Domain. See bottom of file for details.

ferr_hash.h

	This is a collection of hashing functions that I use in various projects.
	It's mostly an effort to consolidate and clean the stuff I've scattered
	around my code!

	Included is an FNV-1a hash that I use often, plus a compile-time variant
	that can be great for avoiding runtime hashing costs. They don't match
	eachother, but I added a runtime variant that does match. Just in case.

	Works on strings and data chunks.
	
Example usage:

	struct test_t {
		float x, y, z;
	};

	const char *str = "test a hash!";

	uint32_t h_const = hashc_constfnv32_string("test a hash!"); // Compile time: 3166176921
	uint32_t h_dyn   = hash_constfnv32_string(str);             // Runtime:      3166176921
	uint32_t h_fnv   = hash_fnv32_string(str);                  // 1193034313

	uint64_t h64_const = hashc_constfnv64_string("test a hash!"); // Compile time: 9942215223050522873
	uint64_t h64_dyn   = hash_constfnv64_string(str);             // Runtime:      9942215223050522873
	uint64_t h64_fnv   = hash_fnv64_string(str);                  // 15918807425826589481

	test_t tmp = { 10, 1, 10 };
	uint32_t h_struct1 = hash_fnv32_data(&tmp, sizeof(test_t));  // 1650533608

	test_t tmp2 = { 10, 1, 10 };
	uint32_t h_struct2 = hash_fnv32_data(&tmp2, sizeof(test_t)); // 1650533608
*/

#pragma once

#include <stdint.h>

#ifdef __cplusplus
#define FERR_HASH_DEFAULT(x) = x
#else
#define FERR_HASH_DEFAULT(x)
#endif

///////////////////////////////////////////
//             FNV-1a hash!              //
///////////////////////////////////////////
// See: http://isthe.com/chongo/tech/comp/fnv/
// for details about FNV-1a

#define HASH_FNV32_START 2166136261
#define HASH_FNV64_START 14695981039346656037

uint64_t hash_fnv64_string(const char *string,                 uint64_t start_hash FERR_HASH_DEFAULT( HASH_FNV64_START ));
uint64_t hash_fnv64_data  (const void *data, size_t data_size, uint64_t start_hash FERR_HASH_DEFAULT( HASH_FNV64_START ));

uint32_t hash_fnv32_string(const char *string,                 uint32_t start_hash FERR_HASH_DEFAULT( HASH_FNV32_START ));
uint32_t hash_fnv32_data  (const void *data, size_t data_size, uint32_t start_hash FERR_HASH_DEFAULT( HASH_FNV32_START ));

///////////////////////////////////////////
//         Compile time hash!            //
///////////////////////////////////////////
//
// hash64c_constfnv_string and hash32c_constfnv_string execute at compile
// time, so you don't have to worry about cost to calculate during runtime.
// They're based on FNV-1a but don't produce the same results. They also have
// a limit of 64 characters. Characters after that will be ignored.
// See http://lolengine.net/blog/2011/12/20/cpp-constant-string-hash for
// details about this implementation.
//
// You can also use hash32_constfnv_string or hash64_constfnv_string to run
// the exact same hash at runtime, if you need to compare a dynamic string 
// with one built at compile time. It's slower than the regular fnv hash too.

#ifndef hash32c_constfnv_string
#define _h32_H1(s,i,x)  (((x)^s[(i)<sizeof(s)?(i):sizeof(s)-1])*16777619u)
#define _h32_H4(s,i,x)  _h32_H1 (s,i+3, _h32_H1 (s,i+2, _h32_H1 (s,i+1 ,_h32_H1 (s,i,x))))
#define _h32_H16(s,i,x) _h32_H4 (s,i+12,_h32_H4 (s,i+8, _h32_H4 (s,i+4, _h32_H4 (s,i,x))))
#define _h32_H64(s,i,x) _h32_H16(s,i+48,_h32_H16(s,i+32,_h32_H16(s,i+16,_h32_H16(s,i,x))))
#define hashc_constfnv32_string(str) ((uint32_t)(_h32_H64(str,0,HASH_FNV32_START)))
#define hashc_constfnv32_type(type) hash32c_constfnv_string(#type)

#define _h64_H1(s,i,x)  (((x)^s[(i)<sizeof(s)?(i):sizeof(s)-1])*1099511628211Ui64)
#define _h64_H4(s,i,x)  _h64_H1 (s,i+3, _h64_H1 (s,i+2, _h64_H1 (s,i+1, _h64_H1 (s,i,x))))
#define _h64_H16(s,i,x) _h64_H4 (s,i+12,_h64_H4 (s,i+8, _h64_H4 (s,i+4, _h64_H4 (s,i,x))))
#define _h64_H64(s,i,x) _h64_H16(s,i+48,_h64_H16(s,i+32,_h64_H16(s,i+16,_h64_H16(s,i,x))))
#define hashc_constfnv64_string(str) ((uint64_t)(_h64_H64(str,0,HASH_FNV64_START)))
#define hashc_constfnv64_type(type) hash64c_constfnv_string(#type)
#endif

uint64_t hash_constfnv64_string(const char *string);
uint32_t hash_constfnv32_string(const char *string);

///////////////////////////////////////////

#ifdef FERR_HASH_IMPL

///////////////////////////////////////////

// Creates a 64 bit hash from a string. Use start_hash with a previous hash if you want to chain hashes together.
uint64_t  hash_fnv64_string(const char* string, uint64_t start_hash) {
	uint64_t hash = start_hash;
	uint8_t  c;
	while (c = *string++)
		hash = (hash ^ c) * 1099511628211;
	return hash;
}

///////////////////////////////////////////

// Creates a 64 bit hash from a chunk of bytes. Use start_hash with a previous hash if you want to chain hashes together.
uint64_t hash_fnv64_data(const void *data, size_t data_size, uint64_t start_hash) {
	uint64_t hash = start_hash;
	uint8_t *bytes = (uint8_t *)data;
	for (size_t i = 0; i < data_size; i++)
		hash = (hash ^ bytes[i]) * 1099511628211;
	return hash;
}

///////////////////////////////////////////

// Creates a 32 bit hash from a string. Use start_hash with a previous hash if you want to chain hashes together.
uint32_t hash_fnv32_string(const char* string, uint32_t start_hash) {
	uint32_t hash = start_hash;
	uint8_t  c;
	while (c = *string++)
		hash = (hash ^ c) * 16777619;
	return hash;
}

///////////////////////////////////////////

// Creates a 32 bit hash from a chunk of bytes. Use start_hash with a previous hash if you want to chain hashes together.
uint32_t hash_fnv32_data(const void *data, size_t data_size, uint32_t start_hash) {
	uint32_t hash = start_hash;
	uint8_t *bytes = (uint8_t *)data;
	for (size_t i = 0; i < data_size; i++)
		hash = (hash ^ bytes[i]) * 16777619;
	return hash;
}

///////////////////////////////////////////

uint64_t hash_constfnv64_string(const char* string) {
	uint64_t hash = HASH_FNV64_START;
	uint8_t  c = string[0];
	for (size_t i = 0; i < 64; i++) {
		if (c != 0) c = string[i];
		hash = (hash ^ c) * 1099511628211;
	}
	return hash;
}

///////////////////////////////////////////

uint32_t hash_constfnv32_string(const char* string) {
	uint32_t hash = HASH_FNV32_START;
	uint8_t  c = string[0];
	for (size_t i = 0; i < 64; i++) {
		if (c != 0) c = string[i];
		hash = (hash ^ c) * 16777619;
	}
	return hash;
}

#endif


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