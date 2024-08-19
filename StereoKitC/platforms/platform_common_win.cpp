/* SPDX-License-Identifier: MIT */
/* The authors below grant copyright rights under the MIT license:
 * Copyright (c) 2019-2024 Nick Klingensmith
 * Copyright (c) 2023-2024 Qualcomm Technologies, Inc.
 */

#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif

#include "platform.h"
#if defined (SK_OS_WINDOWS) || defined(SK_OS_WINDOWS_UWP)

#include "../stereokit.h"
#include "../sk_memory.h"
#include "../libraries/array.h"

#include <stdio.h>
#include <string.h>

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

namespace sk {

///////////////////////////////////////////

void platform_print_callstack() { }

///////////////////////////////////////////

void platform_sleep(int ms) {
	Sleep(ms);
}

///////////////////////////////////////////

font_t platform_default_font() {
	array_t<const char *> fonts = array_t<const char *>::make(3);
	fonts.add(platform_file_exists("C:/Windows/Fonts/segoeui.ttf")
		? "C:/Windows/Fonts/segoeui.ttf"
		: "C:/Windows/Fonts/arial.ttf");

	if      (platform_file_exists("C:/Windows/Fonts/YuGothR.ttc")) fonts.add("C:/Windows/Fonts/YuGothR.ttc");
	else if (platform_file_exists("C:/Windows/Fonts/YuGothm.ttc")) fonts.add("C:/Windows/Fonts/YuGothm.ttc");
	else if (platform_file_exists("C:/Windows/Fonts/Meiryo.ttc" )) fonts.add("C:/Windows/Fonts/Meiryo.ttc");
	else if (platform_file_exists("C:/Windows/Fonts/Yumin.ttf"  )) fonts.add("C:/Windows/Fonts/Yumin.ttf");

	if (platform_file_exists("C:/Windows/Fonts/segmdl2.ttf")) fonts.add("C:/Windows/Fonts/segmdl2.ttf");

	font_t result = font_create_files(fonts.data, fonts.count);
	fonts.free();
	return result;
}

///////////////////////////////////////////

char *platform_working_dir() {
	int32_t  len    = GetCurrentDirectoryW(0, nullptr);
	wchar_t* wresult = sk_malloc_t(wchar_t, len);
	GetCurrentDirectoryW(len, wresult);

	char* result = platform_from_wchar(wresult);

	sk_free(wresult);
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

///////////////////////////////////////////

// Constants for the registry key and value names
const wchar_t* REG_KEY_NAME = L"Software\\StereoKit Simulator";

///////////////////////////////////////////

bool platform_key_save_bytes(const char* key, void* data, int32_t data_size) {
	HKEY reg_key = {};
	if (RegCreateKeyExW(HKEY_CURRENT_USER, REG_KEY_NAME, 0, nullptr, 0, KEY_WRITE, nullptr, &reg_key, nullptr) != ERROR_SUCCESS)
		return false;

	wchar_t w_key[64];
	MultiByteToWideChar(CP_UTF8, 0, key, -1, w_key, _countof(w_key));

	RegSetValueExW(reg_key, w_key, 0, REG_BINARY, (const BYTE*)data, data_size);
	RegCloseKey   (reg_key);
	return true;
}

///////////////////////////////////////////

bool platform_key_load_bytes(const char* key, void* ref_buffer, int32_t buffer_size) {
	HKEY reg_key = {};
	if (RegOpenKeyExW(HKEY_CURRENT_USER, REG_KEY_NAME, 0, KEY_READ, &reg_key) != ERROR_SUCCESS)
		return false;

	wchar_t w_key[64];
	MultiByteToWideChar(CP_UTF8, 0, key, -1, w_key, _countof(w_key));

	DWORD data_size = buffer_size;
	RegQueryValueExW(reg_key, w_key, 0, nullptr, (BYTE*)ref_buffer, &data_size);
	RegCloseKey     (reg_key);
	return true;
}

}

#endif