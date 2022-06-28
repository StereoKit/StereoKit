#include "stereokit.h"
#include "sk_memory.h"
#include "log.h"
#include "libraries/stref.h"
#include "libraries/array.h"
#include "platforms/platform_utils.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>

namespace sk {

///////////////////////////////////////////

typedef void(*log_listener_t)(log_, const char *);
array_t<log_listener_t> log_listeners = {};

log_        log_filter = log_diagnostic;
log_colors_ log_colors = log_colors_ansi;

char       *log_fail_reason_str = nullptr;
int32_t     log_fail_confidence = -1;

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
	char *result = sk_malloc_t(char, (len - remove) + (add * code_size));
	char *at     = result;
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
				for (int i = 0; i < count; i++) {
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
	if (level < log_filter)
		return;

	const char *tag = "";
	switch (level) {
	case log_none:                                       return;
	case log_diagnostic: tag = "<~blu>diagnostic<~clr>"; break;
	case log_inform:     tag = "<~cyn>info<~clr>";       break;
	case log_warning:    tag = "<~ylw>warning<~clr>";    break;
	case log_error:      tag = "<~red>error<~clr>";      break;
	default:
		break;
	}

	size_t len       = strlen(tag) + strlen(text) + 10;
	char  *full_text = sk_malloc_t(char, len);
	snprintf(full_text, len, "[SK %s] %s\n", tag, text);

	char *colored_text = log_replace_colors(full_text, log_colorkeys[log_colors], log_colorcodes[log_colors], log_code_count[log_colors], log_code_size[log_colors]);
#if defined(SK_OS_WINDOWS) || defined(SK_OS_LINUX)
	printf("%s", colored_text);
#endif

	// OutputDebugStringA shows up in the VS output, and doesn't display colors at all
	if (log_colors != log_colors_none) {
		sk_free(colored_text);
		colored_text = log_replace_colors(full_text, log_colorkeys[log_colors_none], log_colorcodes[log_colors_none], log_code_count[log_colors_none], log_code_size[log_colors_none]);
	}
	// Send the plain-text version out to the listeners as well
	for (int32_t i = 0; i < log_listeners.count; i++) {
		log_listeners[i](level, colored_text);
	}
	platform_debug_output(level, colored_text);
	sk_free(colored_text);

	sk_free(full_text);

	if (level == log_error)
		platform_print_callstack();
}

///////////////////////////////////////////

void _log_writef(log_ level, const char* text, va_list args) {
	va_list copy;
	va_copy(copy, args);
	size_t length = vsnprintf(nullptr, 0, text, args);
	char*  buffer = sk_malloc_t(char, length + 2);
	vsnprintf(buffer, length + 2, text, copy);

	log_write(level, buffer);
	sk_free(buffer);
	va_end(copy);
}

///////////////////////////////////////////

void log_writef(log_ level, const char *text, ...) {
	va_list args;
	va_start(args, text);
	_log_writef(level, text, args);
	va_end(args);
}

///////////////////////////////////////////

void log_diag(const char* text) { log_write(log_diagnostic, text); }
void log_info(const char* text) { log_write(log_inform,     text); }
void log_warn(const char* text) { log_write(log_warning,    text); }
void log_err (const char* text) { log_write(log_error,      text); }
void log_diagf(const char* text, ...) {
	va_list args;
	va_start(args, text);
	_log_writef(log_diagnostic, text, args);
	va_end(args);
}
void log_infof(const char* text, ...) {
	va_list args;
	va_start(args, text);
	_log_writef(log_inform, text, args);
	va_end(args);
}
void log_warnf(const char* text, ...) {
	va_list args;
	va_start(args, text);
	_log_writef(log_warning, text, args);
	va_end(args);
}
void log_errf (const char* text, ...) {
	va_list args;
	va_start(args, text);
	_log_writef(log_error, text, args);
	va_end(args);
}

///////////////////////////////////////////

void log_set_filter(log_ level) {
	log_filter = level;
}

///////////////////////////////////////////

void log_set_colors(log_colors_ colors) {
	log_colors = colors;
}

///////////////////////////////////////////

void log_fail_reason(int32_t confidence, log_ log_as, const char *fail_reason) {
	log_write(log_as, fail_reason);

	if (confidence <= log_fail_confidence)
		return;

	sk_free(log_fail_reason_str);
	log_fail_confidence = confidence;
	log_fail_reason_str = string_copy(fail_reason);
}

///////////////////////////////////////////

void log_fail_reasonf(int32_t confidence, log_ log_as, const char *fail_reason, ...) {
	va_list args;
	va_start(args, fail_reason);
	va_list copy;
	va_copy(copy, args);
	size_t length = vsnprintf(nullptr, 0, fail_reason, args);
	char*  buffer = sk_malloc_t(char, length + 2);
	vsnprintf(buffer, length + 2, fail_reason, copy);

	log_fail_reason(confidence, log_as, buffer);
	sk_free(buffer);
	va_end(copy);
	va_end(args);
}

///////////////////////////////////////////

void log_show_any_fail_reason() {
	if (log_fail_confidence == -1)
		return;

	platform_msgbox_err(log_fail_reason_str, "StereoKit Failure");
	log_clear_any_fail_reason();
}

///////////////////////////////////////////

void log_clear_any_fail_reason() {
	sk_free(log_fail_reason_str);
	log_fail_confidence = -1;
	log_fail_reason_str = nullptr;
}

///////////////////////////////////////////

void log_subscribe(void (*on_log)(log_, const char*)) {
	log_listeners.add(on_log);
}

///////////////////////////////////////////

void log_unsubscribe(void (*on_log)(log_, const char*)) {
	for (int32_t i = 0; i < log_listeners.count; i++) {
		if (log_listeners[i] == on_log) {
			log_listeners.remove(i);
			break;
		}
	}
}

} // namespace sk
