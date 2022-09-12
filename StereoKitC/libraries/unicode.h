#pragma once

#include <stdint.h>
#include <string.h>

///////////////////////////////////////////

// In Unicode, a CODEPOINT is a full character, we can think of this as a
// single char32_t. In utf8 and utf16, it takes multiple CODEUNITs to represent
// a full codepoint (up to 4 char, and 2 char16_t).
// 
// This code does not currently communicate consistently in these terms, but
// for clarity's sake, should.

char32_t utf8_decode_fast   (const char* utf8_str, const char** out_next_char);
bool     utf8_decode_fast_b (const char* utf8_str, const char** out_next_char, char32_t* out_char);
char32_t utf8_decode        (const char* utf8_str, const char** out_next_char, int32_t* out_err);
int32_t  utf8_encode_units  (char32_t ch);
int32_t  utf8_encode        (char* out, char32_t utf);
int32_t  utf8_encode_append (char* buffer, size_t size, char32_t ch);
bool     utf8_is_start      (char ch);
int32_t  utf16_encode_units (char32_t ch);
char32_t utf16_decode_fast  (const char16_t* utf16_str, const char16_t** out_next);
bool     utf16_decode_fast_b(const char16_t* utf16_str, const char16_t** out_next_char, char32_t* out_char);
int32_t  utf16_encode       (char16_t* out, char32_t utf);
int32_t  utf16_encode_append(char16_t* buffer, size_t size, char32_t ch);
bool     utf16_is_start     (char16_t ch);

size_t          strlen           (const char16_t* str);
size_t          utf_charlen      (const char*     str_utf8);
size_t          utf_charlen      (const char16_t* str_utf16);
const char*     utf_at_char      (const char*     str_utf8,   int32_t index);
const char16_t* utf_at_char      (const char16_t* str_utf16,  int32_t index);
bool            utf_remove_chars (char*           at_utf8,    int32_t codepoint_count);
bool            utf_remove_chars (char16_t*       at_utf16,   int32_t codepoint_count);
char*           utf_advance_chars(char*           from_utf8,  int32_t codepoint_count);
char16_t*       utf_advance_chars(char16_t*       from_utf16, int32_t codepoint_count);
bool            utf_insert_char  (const char*     buffer_utf8_start,  size_t buffer_length, char*     at_utf8,  char32_t character);
bool            utf_insert_char  (const char16_t* buffer_utf16_start, size_t buffer_length, char16_t* at_utf16, char32_t character);