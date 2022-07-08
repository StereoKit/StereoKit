#pragma once

#include "../stereokit.h"

#if   defined(__EMSCRIPTEN__)
	#define SK_OS_WEB
	#define SK_XR_WEBXR
	#define WEB_EXPORT EMSCRIPTEN_KEEPALIVE extern "C"
#elif defined(__ANDROID__)
	#define SK_OS_ANDROID
	#define SK_XR_OPENXR
#elif defined(__linux__)
	#define SK_OS_LINUX
	#define SK_XR_OPENXR
#elif defined(WINDOWS_UWP)
	#define SK_OS_WINDOWS_UWP
	#define SK_XR_OPENXR
#elif defined(_WIN32)
	#define SK_OS_WINDOWS
	#define SK_XR_OPENXR
#endif

#if !defined(NDEBUG)
	#define SK_DEBUG
#endif

#if defined(SK_OS_WINDOWS) || defined(SK_OS_WINDOWS_UWP)
#define platform_path_separator "\\"
#define platform_path_separator_c '\\'
#else
#define platform_path_separator "/"
#define platform_path_separator_c '/'
#endif

#ifndef _countof
#define _countof(array) (sizeof(array) / sizeof(array[0]))
#endif

namespace sk {

void  platform_msgbox_err(const char *text, const char *header);
bool  platform_get_cursor(vec2 &out_pos);
void  platform_set_cursor(vec2 window_pos);
float platform_get_scroll();
void  platform_debug_output(log_ level, const char *text);
void  platform_print_callstack();
void  platform_sleep       (int ms);
font_t platform_default_font();
char *platform_working_dir ();
void  platform_iterate_dir (const char *directory_path, void *callback_data, void (*on_item)(void *callback_data, const char *name, bool file));
char *platform_push_path_ref(char *path, const char *directory);
char *platform_pop_path_ref (char *path);
char *platform_push_path_new(const char *path, const char *directory);
char *platform_pop_path_new (const char *path);

bool  platform_keyboard_available();

bool platform_utils_init();
void platform_utils_update();
void platform_utils_shutdown();

#if defined(SK_OS_WINDOWS) || defined(SK_OS_WINDOWS_UWP)
wchar_t *platform_to_wchar  (const char *utf8_string);
char    *platform_from_wchar(const wchar_t *string);
#endif

}