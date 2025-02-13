/* SPDX-License-Identifier: MIT */
/* The authors below grant copyright rights under the MIT license:
 * Copyright (c) 2019-2023 Nick Klingensmith
 * Copyright (c) 2023 Qualcomm Technologies, Inc.
 */

#pragma once
#include "../stereokit.h"

///////////////////////////////////////////

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

///////////////////////////////////////////

namespace sk {

enum platform_win_type_ {
	platform_win_type_none,
	platform_win_type_creatable,
	platform_win_type_existing,
};

enum platform_evt_ {
	platform_evt_none,
	platform_evt_app_focus,
	platform_evt_key_press,
	platform_evt_key_release,
	platform_evt_mouse_press,
	platform_evt_mouse_release,
	platform_evt_character,
	platform_evt_scroll,
	platform_evt_close,
	platform_evt_resize,
};

enum platform_surface_ {
	platform_surface_none,
	platform_surface_swapchain
};

union platform_evt_data_t {
	app_focus_ app_focus;
	key_       press_release;
	char32_t   character;
	float      scroll;
	struct { int32_t width, height; } resize;
};

typedef struct recti_t {
	int32_t x;
	int32_t y;
	int32_t w;
	int32_t h;
} recti_t;

typedef int32_t platform_win_t;

typedef struct platform_file_attr_t {
	bool file;
	int64_t size;
} platform_file_attr_t;

///////////////////////////////////////////

// Platform lifecycle:
//
//// Initialization
// platform_impl_init()
// [backend]_init()
//
//// Main loop
// platform_impl_step()
// [backend]_step_begin();
// [backend]_step_end();
//
//// Shutdown
// [backend]_shutdown()
// platform_shutdown()

bool platform_init      ();
void platform_shutdown  ();
void platform_step_begin();
void platform_step_end  ();

///////////////////////////////////////////

platform_win_type_ platform_win_type        ();
platform_win_t     platform_win_get_existing(platform_surface_ surface_type);
platform_win_t     platform_win_make        (const char *title, recti_t win_rect, platform_surface_ surface_type);
void               platform_win_destroy     (platform_win_t window);
bool               platform_win_next_event  (platform_win_t window, platform_evt_* out_event, platform_evt_data_t* out_event_data);
recti_t            platform_win_rect        (platform_win_t window);

///////////////////////////////////////////

bool   platform_get_cursor        (vec2 *out_pos);
void   platform_set_cursor        (vec2 window_pos);
float  platform_get_scroll        ();
void   platform_msgbox_err        (const char *text, const char *header);
void   platform_debug_output      (log_ level, const char *text);
void   platform_print_callstack   ();
void   platform_sleep             (int ms);
font_t platform_default_font      ();

bool   platform_xr_keyboard_present();
void   platform_xr_keyboard_show   (bool show);
bool   platform_xr_keyboard_visible();

void   platform_set_window        (void *window);
void   platform_set_window_xam    (void *window);

bool   platform_file_exists       (const char* filename);
bool   platform_file_delete       (const char* filename);
char  *platform_working_dir       ();
void   platform_iterate_dir       (const char *directory_path, void *callback_data, void (*on_item)(void *callback_data, const char *name, const platform_file_attr_t platform_file_attr_t));
char  *platform_push_path_ref     (char       *path, const char *directory);
char  *platform_pop_path_ref      (char       *path);
char  *platform_push_path_new     (const char *path, const char *directory);
char  *platform_pop_path_new      (const char *path);
bool32_t platform_read_file_direct(const char* filename_utf8, void** out_data, size_t* out_size);

bool   platform_key_save_bytes    (const char* key, void* data,       int32_t data_size);
bool   platform_key_load_bytes    (const char* key, void* ref_buffer, int32_t buffer_size);

#if defined(SK_OS_WINDOWS) || defined(SK_OS_WINDOWS_UWP)
wchar_t *platform_to_wchar  (const char    *utf8_string);
char    *platform_from_wchar(const wchar_t *string);
#endif

} // namespace sk