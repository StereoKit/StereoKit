// SPDX-License-Identifier: MIT
// The authors below grant copyright rights under the MIT license:
// Copyright (c) 2019-2024 Nick Klingensmith
// Copyright (c) 2024 Qualcomm Technologies, Inc.

#include "stereokit.h"
#include "sk_memory.h"
#include "log.h"
#include "libraries/stref.h"
#include "libraries/array.h"
#include "platforms/platform.h"

#include <string.h>
#include <stdio.h>
#include <stdarg.h>

#if defined(SK_OS_WINDOWS) || defined(SK_OS_WINDOWS_UWP)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#endif

#if defined(SK_OS_ANDROID)
#include <syslog.h>
#endif

#if defined(SK_OS_WEB)
#include <emscripten.h>
#endif

namespace sk {

///////////////////////////////////////////

struct log_callback_t {
	void(*callback)(void*, log_, const char*);
	void* context;
};
array_t<log_callback_t> log_listeners = {};

log_        log_filter = log_diagnostic;
log_colors_ log_colors = log_colors_ansi;

char       *log_fail_reason_str = nullptr;
int32_t     log_fail_confidence = -1;

const char *log_tags[] = {
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

#define LOG_C_CLEAR "\033[0;;0m"
#define LOG_NONE_CLEAR ""
const char *log_colorcodes[_countof(log_tags)] = {
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

///////////////////////////////////////////

void log_platform_output(log_ level, const char* text);

///////////////////////////////////////////

int32_t log_count_color_tags(const char* ch, int32_t *out_len) {
	*out_len = 0;
	int32_t tags = 0;
	int32_t curr = 0;
	while (*ch != '\0') {
		// Looking for tags that look like this:
		// <~red>red text here<~clr>
		if      (             *ch == '<') curr  = 1;
		else if (curr == 1 && *ch != '~') curr  = 0;
		else if (curr == 5)             { curr  = 0; if (*ch == '>') tags += 1; }
		else if (curr >  0)               curr += 1;

		ch       += 1;
		*out_len += 1;
	}
	return tags;
}

///////////////////////////////////////////

void log_replace_colors(const char *src, char *dest, const char **color_tags, const char **color_codes, int32_t count, int32_t code_size) {
	// New string with new color values
	int32_t curr = 0;
	while (*src != '\0') {
		// Logic while in a tag
		if      (             *src == '<') curr  = 1;
		else if (curr == 1 && *src != '~') curr  = 0;
		else if (curr == 5) {
			curr = 0; 
			if (*src == '>') {
				int key_id = count - 1;
				for (int i = 0; i < count; i++) {
					if (string_startswith(src-3, color_tags[i])) {
						key_id = i;
						break;
					}
				}
				if (code_size != 0) memcpy(dest, color_codes[key_id], code_size);
				dest += code_size;
				src += 1;
				continue;
			}
		}
		else if (curr > 0) curr += 1;

		// Copy the character
		if (curr == 0) {
			*dest = *src;
			dest += 1;
		}
		src += 1;
	}
	*dest = *src;
}

///////////////////////////////////////////

void log_write(log_ level, const char *text) {
	if (level < log_filter)
		return;

	const char* tag   = "";
	const char* color = "";
	switch (level) {
	case log_none: return;
	case log_diagnostic: tag = "diagnostic"; color = log_colorcodes[8];  break;
	case log_inform:     tag = "info";       color = log_colorcodes[12]; break;
	case log_warning:    tag = "warning";    color = log_colorcodes[6];  break;
	case log_error:      tag = "error";      color = log_colorcodes[2];  break;
	default: break;
	}

	int32_t text_len   = 0;
	int32_t color_tags = log_colors == log_colors_ansi ? log_count_color_tags(text, &text_len) : 0;

	// Set up some memory if we have color tags we need to replace
	char* replace_buffer = nullptr;
	if (color_tags > 0)
		replace_buffer = sk_stack_alloc_t(char, (text_len - color_tags*7) + color_tags * 8 + 1);

#if defined(SK_OS_WINDOWS) || defined(SK_OS_LINUX)
	const char* colored_text = text;
	if (color_tags > 0) {
		log_replace_colors(text, replace_buffer, log_tags, log_colorcodes, _countof(log_tags), sizeof(LOG_C_CLEAR) - 1);
		colored_text = replace_buffer;
	}

	printf("[SK %s%s" LOG_C_CLEAR "] %s\n", color, tag, colored_text);
#endif

	// The remaining outputs don't support colors
	const char* plain_text = text;
	if (color_tags > 0) {
		log_replace_colors(text, replace_buffer, log_tags, nullptr, _countof(log_tags), 0);
		plain_text = replace_buffer;
	}
	for (int32_t i = 0; i < log_listeners.count; i++) {
		log_listeners[i].callback(log_listeners[i].context, level, plain_text);
	}
	log_platform_output(level, plain_text);
	if (level == log_error) platform_print_callstack();
}

///////////////////////////////////////////

char *make_variadic_string(const char* text, va_list args) {
	va_list copy;
	va_copy (copy, args);
	size_t length = vsnprintf(nullptr, 0, text, args) + 1;
	char*  buffer = sk_malloc_t(char, length);
	vsnprintf(buffer, length, text, copy);
	va_end(copy);

	return buffer;
}

///////////////////////////////////////////

void log_writef(log_ level, const char *text, ...) {
	va_list args;
	va_start(args, text);
	char* buffer = make_variadic_string(text, args);
	va_end(args);

	log_write(level, buffer);
	sk_free(buffer);
}

///////////////////////////////////////////

void log_diag(const char* text) { log_write(log_diagnostic, text); }
void log_info(const char* text) { log_write(log_inform,     text); }
void log_warn(const char* text) { log_write(log_warning,    text); }
void log_err (const char* text) { log_write(log_error,      text); }
void log_diagf(const char* text, ...) {
	va_list args;
	va_start(args, text);
	char* buffer = make_variadic_string(text, args);
	va_end(args);

	log_write(log_diagnostic, buffer);
	sk_free(buffer);
}
void log_infof(const char* text, ...) {
	va_list args;
	va_start(args, text);
	char* buffer = make_variadic_string(text, args);
	va_end(args);

	log_write(log_inform, buffer);
	sk_free(buffer);
}
void log_warnf(const char* text, ...) {
	va_list args;
	va_start(args, text);
	char* buffer = make_variadic_string(text, args);
	va_end(args);

	log_write(log_warning, buffer);
	sk_free(buffer);
}
void log_errf (const char* text, ...) {
	va_list args;
	va_start(args, text);
	char* buffer = make_variadic_string(text, args);
	va_end(args);

	log_write(log_error, buffer);
	sk_free(buffer);
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

void log_subscribe(void (*log_callback)(void* context, log_ level, const char* text), void* context) {
	log_callback_t item = {};
	item.callback = log_callback;
	item.context  = context;
	log_listeners.add(item);
}

///////////////////////////////////////////

void log_unsubscribe(void (*log_callback)(void* context, log_ level, const char* text), void* context) {
	for (int32_t i = 0; i < log_listeners.count; i++) {
		if (log_listeners[i].callback == log_callback &&
			log_listeners[i].context  == context) {
			log_listeners.remove(i);
			break;
		}
	}
}

///////////////////////////////////////////

void log_clear_subscribers() {
	log_listeners.free();
}

///////////////////////////////////////////
// Platform specific logging code        //
///////////////////////////////////////////

#if defined(SK_OS_ANDROID)
static bool log_opened = false;
static char log_name[64];
#endif

void log_set_name(const char* name) {
#if defined(SK_OS_ANDROID)
	if (log_opened) {
		closelog();
	}
	log_opened = true;
	// openlog does not seem to copy the string, it uses _our_ memory, so we
	// need to ensure that memory stays alive.
	snprintf(log_name, sizeof(log_name), "%s", name);
	openlog (log_name, LOG_CONS | LOG_NOWAIT, LOG_USER);
#endif
}

///////////////////////////////////////////

void log_platform_output(log_ level, const char *text) {
#if defined(SK_OS_ANDROID)
	if (log_opened == false) {
		log_opened = true;
		openlog("StereoKit", LOG_CONS | LOG_NOWAIT, LOG_USER);
	}

	int32_t priority = LOG_INFO;
	if      (level == log_diagnostic) priority = LOG_DEBUG;
	else if (level == log_inform    ) priority = LOG_INFO;
	else if (level == log_warning   ) priority = LOG_WARNING;
	else if (level == log_error     ) priority = LOG_ERR;
	syslog(priority, "%s", text);
#elif defined(SK_OS_WINDOWS) || defined(SK_OS_WINDOWS_UWP)
	const char* tag = "";
	switch (level) {
	case log_diagnostic: tag = "diagnostic"; break;
	case log_inform:     tag = "info";       break;
	case log_warning:    tag = "warning";    break;
	case log_error:      tag = "error";      break;
	default: break;
	}

	size_t expand_size = strlen(text) + _countof("[SK diagnostic] \n");
	char*  expanded    = sk_stack_alloc_t(char, expand_size);
	snprintf(expanded, expand_size, "[SK %s] %s\n", tag, text);

	OutputDebugStringA(expanded);
#elif defined(SK_OS_WEB)
	if      (level == log_diagnostic) emscripten_console_log(text);
	else if (level == log_inform    ) emscripten_console_log(text);
	else if (level == log_warning   ) emscripten_console_warn(text);
	else if (level == log_error     ) emscripten_console_error(text);
#endif
}

///////////////////////////////////////////

} // namespace sk
