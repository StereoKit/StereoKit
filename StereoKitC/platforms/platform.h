/* SPDX-License-Identifier: MIT */
/* The authors below grant copyright rights under the MIT license:
 * Copyright (c) 2019-2024 Nick Klingensmith
 * Copyright (c) 2023-2024 Qualcomm Technologies, Inc.
 */

#pragma once
#include "../stereokit.h"

///////////////////////////////////////////

#if defined(__ANDROID__)
	#define SK_OS_ANDROID
	#define SK_XR_OPENXR
#elif defined(__APPLE__)
	#define SK_OS_MACOS
	#define SK_XR_OPENXR
#elif defined(__linux__)
	#define SK_OS_LINUX
	#define SK_XR_OPENXR
#elif defined(_WIN32)
	#define SK_OS_WINDOWS
	#define SK_XR_OPENXR
#endif

#if !defined(NDEBUG)
	#define SK_DEBUG
#endif

#if defined(SK_OS_WINDOWS)
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

void   platform_msgbox_err        (const char *text, const char *header);
void   platform_print_callstack   ();
font_t platform_default_font      ();

void   platform_set_window        (void *window);
void   platform_set_window_xam    (void *window);

bool   platform_file_delete       (const char* filename);
char  *platform_push_path_ref     (char       *path, const char *directory);
char  *platform_pop_path_ref      (char       *path);
char  *platform_push_path_new     (const char *path, const char *directory);
char  *platform_pop_path_new      (const char *path);

bool32_t platform_read_file_direct(const char *filename, void **out_data, size_t *out_size);

} // namespace sk
