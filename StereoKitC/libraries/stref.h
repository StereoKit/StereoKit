#pragma once

#include <stdint.h>
#include <stddef.h>

#define STREF_HASH_START 14695981039346656037UL

struct stref_t {
	const char *start;
	uint32_t    length;

	char temp_end;
	bool temp;
};

char *string_copy      (const char *string);
char *string_make      (stref_t &ref);
char *string_append    (char *base, uint32_t count, ...);
char *string_substr    (const char *a, uint32_t start, uint32_t length);
bool  string_eq        (const char *a, const char *b);
bool  string_eq_nocase (const char *a, const char *b);
bool  string_endswith  (const char *a, const char *end, bool case_sensitive = true);
bool  string_startswith(const char *a, const char *is);

bool     stref_equals  (const stref_t &ref, const char *is);
bool     stref_equals  (const stref_t &a, const stref_t &b);
bool     stref_startswith(const stref_t &a, const char *is);
int32_t  stref_indexof (const stref_t &ref, char character);
int32_t  stref_lastof  (const stref_t &ref, char character);
char    *stref_copy    (const stref_t &ref);
void     stref_copy_to (const stref_t &ref, char *text, size_t text_size);
const char *stref_withend(stref_t &ref);
void     stref_remend  (stref_t &ref);
stref_t  stref_make    (const char *source);
stref_t  stref_substr  (const char *source, uint32_t length);
stref_t  stref_substr  (const stref_t &ref, uint32_t start, uint32_t length);
void     stref_trim    (stref_t &ref);
uint32_t stref_count   (stref_t &ref, char character);
bool     stref_nextline(stref_t &from, stref_t &curr_line);
bool     stref_nextword(stref_t &line, stref_t &word, char separator=' ', char capture_char_start = '\0', char capture_char_end = '\0', bool *out_capture_error = nullptr);
stref_t  stref_stripcapture(stref_t &word, char capture_char_start, char capture_char_end);
float    stref_to_f    (const stref_t &ref);
int32_t  stref_to_i    (const stref_t &ref);

void stref_file_path(const stref_t &filename, stref_t &out_path, stref_t &out_name);
