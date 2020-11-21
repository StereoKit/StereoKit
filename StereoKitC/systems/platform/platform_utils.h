#pragma once

#include "../../stereokit.h"

#if   defined(__EMSCRIPTEN__)
	#define SK_OS_WEB
#elif defined(__ANDROID__)
	#define SK_OS_ANDROID
#elif defined(__linux__)
	#define SK_OS_LINUX
#elif defined(WINDOWS_UWP)
	#define SK_OS_WINDOWS_UWP
#elif defined(_WIN32)
	#define SK_OS_WINDOWS
#endif

#ifndef _countof
#define _countof(array) (sizeof(array) / sizeof(array[0]))
#endif

namespace sk {

void  platform_msgbox_err(const char *text, const char *header);
bool  platform_read_file (const char *filename, void **out_data, size_t *out_size);
bool  platform_get_cursor(vec2 &out_pos);
void  platform_set_cursor(vec2 window_pos);
float platform_get_scroll();
bool  platform_key_down  (key_ key);
void  platform_debug_output(log_ level, const char *text);
void  platform_sleep     (int ms);

const char *platform_default_font();

}