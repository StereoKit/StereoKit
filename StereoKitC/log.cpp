#include "stereokit.h"
#include "_stereokit.h"
#include "stref.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

///////////////////////////////////////////

log_        log_filter = log_info;
log_colors_ log_colors = log_colors_ansi;

const char *log_colorkeys_c[] = {
	"blk",
	"BLK",
	"red",
	"RED",
	"grn",
	"GRN",
	"ylw",
	"YLW",
	"blu",
	"BLU",
	"mag",
	"MAG",
	"cyn",
	"CYN",
	"wht",
	"WHT",
	"clr",
};
const char *log_colorkeys_none[]{
	"   ",
};

#define LOG_C_CLEAR "\033[0;;0m"
#define LOG_NONE_CLEAR ""
const char *log_colorcodes_c[_countof(log_colorkeys_c)] = {
	"\033[0;30m",
	"\033[1;30m",
	"\033[0;31m",
	"\033[1;31m",
	"\033[0;32m",
	"\033[1;32m",
	"\033[0;33m",
	"\033[1;33m",
	"\033[0;34m",
	"\033[1;34m",
	"\033[0;35m",
	"\033[1;35m",
	"\033[0;36m",
	"\033[1;36m",
	"\033[0;37m",
	"\033[1;37m",
	LOG_C_CLEAR,
};
const char *log_colorcodes_none[_countof(log_colorkeys_none)] = {
	LOG_NONE_CLEAR,
};

const char **log_colorkeys [] = { log_colorkeys_c,  log_colorkeys_none  };
const char **log_colorcodes[] = { log_colorcodes_c, log_colorcodes_none };
const int    log_code_count[] = { _countof(log_colorkeys_c), _countof(log_colorkeys_none) };
const int    log_code_size [] = { _countof(LOG_C_CLEAR),     _countof(LOG_NONE_CLEAR) };


///////////////////////////////////////////

char *log_replace_colors(const char *text, const char **color_keys, const char **color_codes, int count, int code_size) {
	// Looking for words that look like this: 
	// <~red>red text here<~clr>

	const char *ch = text;
	size_t len = strlen(text);

	// Parse for memory allocation
	int remove = 0;
	int add    = 0;
	int curr   = 0;
	while (*ch != '\0') {
		if (curr == 1 && *ch != '~') // make sure second character matches key
			curr = 0;

		if (curr > 0) { // We're in a key
			curr += 1;
			if (*ch == '>') { // End of a key, make note
				add += 1;
				remove += curr;
				curr = 0;
			}
		}
		if (*ch == '<') curr = 1; // Start a color key

		ch++;
	}

	// No keys found, just use the original string!
	if (add == 0)
		return nullptr;

	// New string with new color values
	char *result = (char*)malloc((len - remove) + (add * code_size));
	char *at = result;
	ch   = text;
	curr = 0;
	char key[4] = "   ";
	while (*ch != '\0') {
		if (curr == 1 && *ch != '~') // make sure second character matches key
			curr = 0;
		if (curr > 0) { // We're in a key
			curr += 1;

			if (curr > 2 && curr < 6) { // text of key
				key[curr - 3] = *ch;
			}
			if (*ch == '>') {
				int key_id = count-1;
				for (size_t i = 0; i < count; i++) {
					if (string_eq(key, color_keys[i])) {
						key_id = i;
						break;
					}
				}
				memcpy(at, color_codes[key_id], code_size-1);
				at += code_size-1;
				curr = 0;
				ch++;
				continue;
			}
		}
		if (*ch == '<') curr = 1; // Start a color key

		// Copy the character as is
		if (curr == 0) {
			*at = *ch;
			at++;
		}
		ch++;
	}
	*at = *ch;
	return result;
}

///////////////////////////////////////////

void log_write(log_ level, const char *text) {
	bool no_colors = log_colors == log_colors_none;

	const char *tag = "";
	switch (level) {
	case log_info:    tag = no_colors ? "info"    : "\033[0;36minfo\033[0;;0m";    break;
	case log_warning: tag = no_colors ? "warning" : "\033[0;33mwarning\033[0;;0m"; break;
	case log_error:   tag = no_colors ? "error"   : "\033[0;31merror\033[0;;0m";   break;
	default:
		break;
	}

	char *colored_text = log_replace_colors(text, log_colorkeys[log_colors], log_colorcodes[log_colors], log_code_count[log_colors], log_code_size[log_colors]);
	if (level >= log_filter)
		printf("[SK %s] %s\n", tag, colored_text == nullptr ? text : colored_text);
	free(colored_text);
}

///////////////////////////////////////////

void log_writef(log_ level, const char *text, ...) {
	va_list argptr;
	va_start(argptr, text);
	size_t length = vsnprintf(nullptr, 0, text, argptr);
	char  *buffer = (char*)malloc(length+2);
	vsnprintf(buffer, length+2, text, argptr);
	va_end(argptr);

	log_write(level, buffer);
}

///////////////////////////////////////////

void log_set_filter(log_ level) {
	log_filter = level;
}

///////////////////////////////////////////

void log_set_colors(log_colors_ colors) {
	log_colors = colors;
}