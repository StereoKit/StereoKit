/* SPDX-License-Identifier: MIT */
/* The authors below grant copyright rights under the MIT license:
 * Copyright (c) 2019-2026 Nick Klingensmith
 * Copyright (c) 2023-2026 Qualcomm Technologies, Inc.
 */

#include "platform.h"
#if defined (SK_OS_WINDOWS)

#include "../stereokit.h"
#include "../sk_memory.h"
#include "../libraries/array.h"
#include "../asset_types/font.h"

#include <sk_app.h>

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

namespace sk {

///////////////////////////////////////////

void platform_print_callstack() { }

///////////////////////////////////////////

font_t platform_default_font() {
	array_t<const char *> fonts = array_t<const char *>::make(3);
	if      (ska_file_exists("C:/Windows/Fonts/segoeui.ttf")) fonts.add("C:/Windows/Fonts/segoeui.ttf");
	else if (ska_file_exists("C:/Windows/Fonts/arial.ttf"))   fonts.add("C:/Windows/Fonts/arial.ttf");

	if      (ska_file_exists("C:/Windows/Fonts/YuGothR.ttc")) fonts.add("C:/Windows/Fonts/YuGothR.ttc");
	else if (ska_file_exists("C:/Windows/Fonts/YuGothm.ttc")) fonts.add("C:/Windows/Fonts/YuGothm.ttc");
	else if (ska_file_exists("C:/Windows/Fonts/Meiryo.ttc" )) fonts.add("C:/Windows/Fonts/Meiryo.ttc");
	else if (ska_file_exists("C:/Windows/Fonts/Yumin.ttf"  )) fonts.add("C:/Windows/Fonts/Yumin.ttf");

	if (ska_file_exists("C:/Windows/Fonts/segmdl2.ttf")) fonts.add("C:/Windows/Fonts/segmdl2.ttf");

	if (fonts.count <= 0)
		return font_create_default();

	font_t result = font_create_files(fonts.data, fonts.count);
	fonts.free();
	return result;
}

///////////////////////////////////////////

wchar_t *platform_to_wchar(const char *utf8_string) {
	int32_t  wsize  = MultiByteToWideChar(CP_UTF8, 0, utf8_string, -1, nullptr, 0);
	wchar_t *result = sk_malloc_t(wchar_t, wsize);
	MultiByteToWideChar(CP_UTF8, 0, utf8_string, -1, result, wsize);
	return result;
}

///////////////////////////////////////////

char *platform_from_wchar(const wchar_t *string) {
	int32_t len  = (int)(wcslen(string)+1);
	int32_t size = WideCharToMultiByte(CP_UTF8, 0, string, len, nullptr, 0, nullptr, nullptr);
	char *result = sk_malloc_t(char, size);
	WideCharToMultiByte               (CP_UTF8, 0, string, len, result, size, nullptr, nullptr);
	return result;
}

}

#endif
