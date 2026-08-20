/* SPDX-License-Identifier: MIT */
/* The authors below grant copyright rights under the MIT license:
 * Copyright (c) 2019-2024 Nick Klingensmith
 * Copyright (c) 2023-2024 Qualcomm Technologies, Inc.
 */

#pragma once
#include "../stereokit.h"

///////////////////////////////////////////

// Emscripten also defines __linux__, so it has to be tested first
#if defined(__EMSCRIPTEN__)
	#define SK_OS_WEB
#elif defined(__ANDROID__)
	#define SK_OS_ANDROID
#elif defined(__APPLE__)
	#define SK_OS_MACOS
#elif defined(__linux__)
	#define SK_OS_LINUX
#elif defined(_WIN32)
	#define SK_OS_WINDOWS
#endif

// The OpenXR backend imports XR swapchain images as Vulkan textures, so it only
// exists on the Vulkan backend. CMake matches this with SK_ENABLE_OPENXR.
#if !defined(SKR_WEBGPU)
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

typedef struct ska_window_t ska_window_t;

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

// The windowed backends hand their window over here, and the public window_*
// API drives it without knowing which backend is running.
void   platform_set_active_window (ska_window_t *window);

bool   platform_file_delete       (const char* filename);
bool   platform_asset_exists      (const char* filename);
char  *platform_push_path_ref     (char       *path, const char *directory);
char  *platform_pop_path_ref      (char       *path);
char  *platform_push_path_new     (const char *path, const char *directory);
char  *platform_pop_path_new      (const char *path);

bool32_t platform_read_file_direct(const char *filename, void **out_data, size_t *out_size);
size_t   platform_file_size       (const char *filename);

typedef void (*platform_read_callback_t)(bool32_t success, void *data, size_t size, void *context);
// Reads a file without blocking the caller; the callback owns `data`. Off the
// web the callback runs before this returns. On the web a file the page
// preloaded does the same, and anything else streams in from the server,
// landing on the main thread later.
void platform_read_file_async(const char *filename, platform_read_callback_t callback, void *context);

} // namespace sk
