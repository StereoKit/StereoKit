#include "stereokit.h"
#include <stdio.h>
#include <stdarg.h>

///////////////////////////////////////////

log_ log_filter = log_info;

///////////////////////////////////////////

void log_write(log_ level, const char *text) {
	const char *tag = "";
	switch (level) {
	case log_info:    tag = "info"; break;
	case log_warning: tag = "warning"; break;
	case log_error:   tag = "error"; break;
	default:
		break;
	}
	if (level >= log_filter)
		printf("[SK %s] %s\n", tag, text);
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