/* SPDX-License-Identifier: MIT */
/* The authors below grant copyright rights under the MIT license:
 * Copyright (c) 2019-2023 Nick Klingensmith
 * Copyright (c) 2023 Qualcomm Technologies, Inc.
 */

#include "win32.h"
#if defined(SK_OS_WINDOWS)

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <shellapi.h>

#include "../stereokit.h"
#include "../sk_memory.h"
#include "../libraries/stref.h"

namespace sk {

///////////////////////////////////////////

HINSTANCE win32_hinst = nullptr;
HICON     win32_icon  = nullptr;

///////////////////////////////////////////

bool platform_impl_init() {
	win32_hinst = GetModuleHandle(NULL);

	// Find the icon from the exe itself
	wchar_t path[MAX_PATH];
	if (GetModuleFileNameW(GetModuleHandle(nullptr), path, MAX_PATH) != 0)
		ExtractIconExW(path, 0, &win32_icon, nullptr, 1);

	return true;
}

///////////////////////////////////////////

void platform_impl_shutdown() {
	win32_hinst = nullptr;
	win32_icon  = nullptr;
}

///////////////////////////////////////////

void platform_impl_step() {
}

///////////////////////////////////////////

void platform_msgbox_err(const char* text, const char* header) {
	wchar_t* text_w   = platform_to_wchar(text);
	wchar_t* header_w = platform_to_wchar(header);
	MessageBoxW(nullptr, text_w, header_w, MB_OK | MB_ICONERROR);
	sk_free(text_w);
	sk_free(header_w);
}

} // namespace sk

#endif // defined(SK_OS_WINDOWS)
