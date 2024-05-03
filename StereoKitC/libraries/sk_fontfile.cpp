// SPDX-License-Identifier: MIT
// The authors below grant copyright rights under the MIT license:
// Copyright (c) 2024 Nick Klingensmith
// Copyright (c) 2024 Qualcomm Technologies, Inc.

#include "sk_fontfile.h"

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <ctype.h>

///////////////////////////////////////////
// OS Independant                        //
///////////////////////////////////////////

char* _wchar_to_utf8(const wchar_t* string) {
	size_t len    = wcstombs(nullptr, string, 0) + 1;
	char*  result = (char*)malloc(len);
	wcstombs(result, string, len);
	return result;
}

///////////////////////////////////////////

char* _trim_whitespace(const char* string) {
	const char* start   = string;
	const char* end     = string + strlen(string) - 1;
	while (               isspace((unsigned char)*start)) start++;
	while (end > start && isspace((unsigned char)*end  )) end--;

	size_t len     = (end - start) + 1;
	char*  trimmed = (char*)malloc(len+1);
	if (trimmed == nullptr) return nullptr;
	memcpy(trimmed, start, len);
	trimmed[len] = '\0';
	return trimmed;
}

///////////////////////////////////////////

void fontfile_print_font(const char* name) {
	font_fallback_info_t* info;
	int32_t               info_count;
	fontfile_from_css(name, &info, &info_count);
	if (info_count == 0) {
	}
	printf("Font Selection: \"%s\"\n|Base: %s -> %s\n", name, info[0].name, info[0].filename);
	for (int32_t i = 1; i < info_count; i++) {
		if (info[i].scale != 1) printf("|--\"%s\" -> %s %.2f\n", info[i].name, info[i].filename, info[i].scale);
		else                    printf("|--\"%s\" -> %s\n", info[i].name, info[i].filename);
	}
	free(info);
}

///////////////////////////////////////////
// Windows                               //
///////////////////////////////////////////

#ifdef _WIN32

#include <windows.h>
#include <shlobj.h>

///////////////////////////////////////////

char* fontfile_folder() {
	// We have a valid font name now, prefix the windows font folder to it.
	wchar_t font_folder_path[MAX_PATH];
	SHGetFolderPathW(nullptr, CSIDL_FONTS, nullptr, SHGFP_TYPE_CURRENT, font_folder_path);
	return _wchar_to_utf8(font_folder_path);
}

///////////////////////////////////////////

char* fontfile_name_to_path(const char* name_utf8) {
	// Filenames are already filenames
	if (strchr(name_utf8, '.') != nullptr) {
		return _strdup(name_utf8);
	}

	wchar_t name[256];
	swprintf(name, sizeof(name) / sizeof(wchar_t), L"%hs", name_utf8);

	// Check to see if this font has a substitute
	HKEY  substitute_key;
	DWORD err = RegOpenKeyExW(HKEY_LOCAL_MACHINE,
		L"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\FontSubstitutes",
		0, KEY_READ, &substitute_key);
	if (err == ERROR_SUCCESS) {
		wchar_t substitute_name[512];
		DWORD   substitute_size = sizeof(substitute_name);
		err = RegQueryValueExW(substitute_key, name, nullptr, nullptr, (LPBYTE)substitute_name, &substitute_size);
		if (err == ERROR_SUCCESS) {
			swprintf(name, sizeof(name) / sizeof(wchar_t), L"%s", substitute_name);
		}
		RegCloseKey(substitute_key);
	}

	// Acquire the font file for this name
	HKEY key;
	err = RegOpenKeyExW(HKEY_LOCAL_MACHINE,
		L"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Fonts",
		0, KEY_READ, &key);
	if (err != ERROR_SUCCESS) return nullptr;

	wchar_t font_key[256];
	swprintf(font_key, sizeof(font_key) / sizeof(wchar_t), L"%s (TrueType)", name);

	wchar_t font_file_name[512];
	DWORD   buffer_size = sizeof(font_file_name);
	err = RegQueryValueExW(key, font_key, nullptr, nullptr, (LPBYTE)font_file_name, &buffer_size);

	RegCloseKey(key);

	if (err != ERROR_SUCCESS) return nullptr;

	return _wchar_to_utf8(font_file_name);
}

///////////////////////////////////////////

bool fontfile_get_fallback_info(const char* name_utf8, font_fallback_info_t** out_info, int32_t* out_info_count) {
	*out_info       = nullptr;
	*out_info_count = 0;

	// Filenames have no fallback
	if (strchr(name_utf8, '.') != nullptr)
		return false;

	wchar_t name[256];
	swprintf(name, sizeof(name) / sizeof(wchar_t), L"%hs", name_utf8);

	// Check to see if this font has a substitute
	HKEY  substitute_key;
	DWORD err = RegOpenKeyExW(HKEY_LOCAL_MACHINE,
		L"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\FontSubstitutes",
		0, KEY_READ, &substitute_key);
	if (err == ERROR_SUCCESS) {
		wchar_t substitute_name[512];
		DWORD   substitute_size = sizeof(substitute_name);
		err = RegQueryValueExW(substitute_key, name, nullptr, nullptr, (LPBYTE)substitute_name, &substitute_size);
		if (err == ERROR_SUCCESS) {
			swprintf(name, sizeof(name) / sizeof(wchar_t), L"%s", substitute_name);
		}
		RegCloseKey(substitute_key);
	}

	// Acquire the font file for this name
	HKEY key;
	err = RegOpenKeyExW(HKEY_LOCAL_MACHINE,
		L"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\FontLink\\SystemLink",
		0, KEY_READ, &key);
	if (err != ERROR_SUCCESS) return false;

	DWORD buffer_size = 0;
	err = RegQueryValueExW(key, name, nullptr, nullptr, nullptr, &buffer_size);
	if (err != ERROR_SUCCESS) { RegCloseKey(key); return false; }
	wchar_t* buffer = (wchar_t* )malloc(buffer_size);
	err = RegQueryValueExW(key, name, nullptr, nullptr, (LPBYTE)buffer, &buffer_size);
	RegCloseKey(key);
	if (err != ERROR_SUCCESS) { free(buffer); return false; }

	// Allocate memory for our list, based on the maximum number of potential
	// entries.
	int32_t  max_count = 0;
	wchar_t* line      = buffer;
	while (line[0] != '\0') {
		max_count += 1;
		line      += wcslen(line) + 1;
	}
	*out_info = (font_fallback_info_t*)malloc(sizeof(font_fallback_info_t) * max_count);
	if (*out_info == nullptr) return false;
	font_fallback_info_t* info = *out_info;

	line = buffer;
	int32_t count = 0;
	while (line[0] != '\0' && count < max_count) {
		wchar_t wfilename[128], wname[128];
		int32_t numerator, denominator;
		int32_t num_fields = swscanf(line, L"%127[^,],%127[^,],%d,%d", wfilename, wname, &numerator, &denominator);
		wfilename[127] = wname[127] = '\0';

		if (num_fields < 3) numerator   = 1;
		if (num_fields < 4) denominator = 1;
		wcstombs(info[count].filename, wfilename, sizeof(info[count].filename));
		wcstombs(info[count].name,     wname,     sizeof(info[count].name));
		info[count].scale = numerator / (float)denominator;

		if (count > 0 && strcmp(info[count - 1].name, info[count].name) == 0) {
			if (info[count - 1].scale == 1)
				info[count - 1].scale = info[count].scale;
		} else {
			count += 1;
		}
		line += wcslen(line) + 1;
	}
	*out_info_count = count;

	free(buffer);

	return true;
}

///////////////////////////////////////////

void fontfile_from_css(const char* fontlist_utf8, font_fallback_info_t** out_info, int32_t* out_info_count) {
	*out_info       = nullptr;
	*out_info_count = 0;

	char* string = _strdup(fontlist_utf8);
	char* token  = strtok(string, ",");

	// Super simple dynamic array
	int32_t               count    = 0;
	int32_t               capacity = 4;
	font_fallback_info_t* data     = (font_fallback_info_t*)malloc(sizeof(font_fallback_info_t) * capacity);
	if (data == nullptr) return;

	while (token != nullptr) {
		char *trimmed = _trim_whitespace(token);
		char *file    = fontfile_name_to_path(trimmed);
		token = strtok(nullptr, ",");

		if (file != nullptr) {
			if (count >= capacity) { capacity = capacity * 2; data = (font_fallback_info_t*)realloc(data, sizeof(font_fallback_info_t) * capacity); }
			if (data == nullptr) return;

			strncpy(data[count].name,     trimmed, sizeof(data[count].name));
			strncpy(data[count].filename, file,    sizeof(data[count].filename));
			data[count].scale = 1;
			count += 1;

			free(file);

			int32_t               link_count;
			font_fallback_info_t* links;
			if (fontfile_get_fallback_info(trimmed, &links, &link_count)) {
				for (size_t i = 0; i < link_count; i++)
				{
					if (count >= capacity) { capacity = capacity * 2; data = (font_fallback_info_t*)realloc(data, sizeof(font_fallback_info_t) * capacity); }
					if (data == nullptr) return;
					
					memcpy(&data[count], &links[i], sizeof(font_fallback_info_t));
					count += 1;
				}
				free(trimmed);
				break;
			}
		}
		free(trimmed);
	}
	free(string);

	*out_info       = data;
	*out_info_count = count;
}

#endif

///////////////////////////////////////////
// Android                               //
///////////////////////////////////////////

#if defined(__ANDROID__)

#include <android/font_matcher.h>
#include <android/font.h>

///////////////////////////////////////////

char* fontfile_folder() {
	// Androids fonts are located here
	return "/system/fonts";
}

///////////////////////////////////////////

char* fontfile_name_to_path(const char* name_utf8) {
	char* font_file_path = nullptr;
	// Filenames are already filenames
	if (strchr(name_utf8, '.') != nullptr) {
		return strdup(name_utf8);
	}
	AFontMatcher* matcher = AFontMatcher_create();
	AFont* font_matched = AFontMatcher_match(matcher, name_utf8, (uint16_t*)u"A", 1, nullptr);
	if (font_matched) {
		font_file_path = strdup(AFont_getFontFilePath(font_matched));
	}
	AFont_close(font_matched);
	AFontMatcher_destroy(matcher);
	return font_file_path;
}

///////////////////////////////////////////

bool fontfile_get_fallback_info(const char* name_utf8, font_fallback_info_t** out_info, int32_t* out_info_count) {
	*out_info = nullptr;
	*out_info_count = 0;

    // There are no fallback fonts in android.
	return false;
}

///////////////////////////////////////////

void fontfile_from_css(const char* fontlist_utf8, font_fallback_info_t** out_info, int32_t* out_info_count) {
	*out_info = nullptr;
	*out_info_count = 0;

	char* string = strdup(fontlist_utf8);
	char* token = strtok(string, ",");

	// Super simple dynamic array
	int32_t               count = 0;
	int32_t               capacity = 4;
	font_fallback_info_t* data = (font_fallback_info_t*)malloc(sizeof(font_fallback_info_t) * capacity);
	if (data == nullptr) return;

	while (token != nullptr) {
		char* trimmed = _trim_whitespace(token);
		char* file = fontfile_name_to_path(trimmed);
		token = strtok(nullptr, ",");

		if (file != nullptr) {
			if (count >= capacity) { capacity = capacity * 2; data = (font_fallback_info_t*)realloc(data, sizeof(font_fallback_info_t) * capacity); }
			if (data == nullptr) return;

			strncpy(data[count].name, trimmed, sizeof(data[count].name));
			strncpy(data[count].filename, file, sizeof(data[count].filename));
			data[count].scale = 1;
			count += 1;

			free(file);

			int32_t               link_count;
			font_fallback_info_t* links;
			if (fontfile_get_fallback_info(trimmed, &links, &link_count)) {
				for (size_t i = 0; i < link_count; i++)
				{
					if (count >= capacity) { capacity = capacity * 2; data = (font_fallback_info_t*)realloc(data, sizeof(font_fallback_info_t) * capacity); }
					if (data == nullptr) return;

					memcpy(&data[count], &links[i], sizeof(font_fallback_info_t));
					count += 1;
				}
				free(trimmed);
				break;
			}
		}
		free(trimmed);
	}
	free(string);

	*out_info = data;
	*out_info_count = count;
}

#endif

///////////////////////////////////////////
// Linux                                 //
///////////////////////////////////////////

#if defined(__linux__) && !defined(__ANDROID__)

#include <fontconfig/fontconfig.h>

///////////////////////////////////////////

char* fontfile_folder() {
	return strdup("/");
}

///////////////////////////////////////////

char* fontfile_name_to_path(const char* name_utf8) {
	// Filenames are already filenames
	if (strchr(name_utf8, '.') != nullptr) {
		return strdup(name_utf8);
	}

	FcConfig*  config  = FcInitLoadConfigAndFonts();
	FcPattern* pattern = FcNameParse((const FcChar8*)name_utf8);
	FcConfigSubstitute (config, pattern, FcMatchPattern);
	FcDefaultSubstitute(pattern);

	char*      file = nullptr;
	FcResult   result;
	FcPattern* font = FcFontMatch(config, pattern, &result);
	if (font) {
		FcChar8* filename;
		if (FcPatternGetString(font, FC_FILE, 0, &filename) == FcResultMatch) {
			file = strdup((const char*)filename);
		}
		FcPatternDestroy(font);
	}
	FcPatternDestroy(pattern);
	FcConfigDestroy (config);

	return file;
}

///////////////////////////////////////////

bool fontfile_get_fallback_info(const char* name_utf8, font_fallback_info_t** out_info, int32_t* out_info_count) {
	*out_info       = nullptr;
	*out_info_count = 0;

	// Filenames have no fallback
	if (strchr(name_utf8, '.') != nullptr)
		return false;

	FcPattern* pattern = FcNameParse((const FcChar8*)name_utf8);
	FcConfig*  config  = FcConfigGetCurrent();
	FcBool     result  = FcConfigSubstitute(config, pattern, FcMatchPattern);
	FcDefaultSubstitute(pattern);

	if (!result) {
		FcPatternDestroy(pattern);
		return false;
	}

	FcResult   fc_result;
	FcFontSet* font_set = FcFontSort(config, pattern, 1, NULL, &fc_result);

	if (!font_set) {
		FcPatternDestroy(pattern);
		return false;
	}

	*out_info_count = font_set->nfont;
	*out_info       = (font_fallback_info_t*)malloc(sizeof(font_fallback_info_t) * font_set->nfont);
	if (*out_info == nullptr) {
		FcFontSetDestroy(font_set);
		FcPatternDestroy(pattern);
		return false;
	}

	for (int i = 0; i < font_set->nfont; i++) {
		FcValue file_value;
		if (FcPatternGet(font_set->fonts[i], FC_FILE, 0, &file_value) == FcResultMatch) {
			strncpy((*out_info)[i].filename, (char*)file_value.u.s, sizeof((*out_info)[i].filename) - 1);
			(*out_info)[i].filename[sizeof((*out_info)[i].filename) - 1] = '\0';
		}

		FcValue nameValue;
		if (FcPatternGet(font_set->fonts[i], FC_FAMILY, 0, &nameValue) == FcResultMatch) {
			strncpy((*out_info)[i].name, (char*)nameValue.u.s, sizeof((*out_info)[i].name) - 1);
			(*out_info)[i].name[sizeof((*out_info)[i].name) - 1] = '\0';
		}

		(*out_info)[i].scale = 1.0;
	}

	FcFontSetDestroy(font_set);
	FcPatternDestroy(pattern);

	return true;
}

///////////////////////////////////////////

void fontfile_from_css(const char* fontlist_utf8, font_fallback_info_t** out_info, int32_t* out_info_count) {
	*out_info       = nullptr;
	*out_info_count = 0;

	char* string = strdup(fontlist_utf8);
	char* token  = strtok(string, ",");

	// Super simple dynamic array
	int32_t               count    = 0;
	int32_t               capacity = 4;
	font_fallback_info_t* data     = (font_fallback_info_t*)malloc(sizeof(font_fallback_info_t) * capacity);
	if (data == nullptr) return;

	FcConfig*  config    = FcInitLoadConfigAndFonts();
	FcPattern* pattern   = FcPatternCreate();

	while (token != nullptr) {
		char *trimmed = _trim_whitespace(token);
		token = strtok(nullptr, ",");

		FcPatternAddString(pattern, FC_FAMILY, (const FcChar8*)trimmed);
		free(trimmed);
	}
	free(string);

	FcConfigSubstitute (config, pattern, FcMatchPattern);
	FcDefaultSubstitute(pattern);

	FcResult   fc_result;
	FcFontSet* font_set = FcFontSort(config, pattern, 1, NULL, &fc_result);

	if (!font_set) {
		FcPatternDestroy(pattern);
		return;
	}

	for (int i = 0; i < font_set->nfont; i++) {
		if (count >= capacity) { capacity = capacity * 2; data = (font_fallback_info_t*)realloc(data, sizeof(font_fallback_info_t) * capacity); }
		if (data == nullptr) return;

		FcValue file_value;
		if (FcPatternGet(font_set->fonts[i], FC_FILE, 0, &file_value) == FcResultMatch) {
			strncpy(data[count].filename, (char*)file_value.u.s, sizeof(data[count].filename) - 1);
			data[count].filename[sizeof(data[count].filename) - 1] = '\0';
		}
		FcValue nameValue;
		if (FcPatternGet(font_set->fonts[i], FC_FAMILY, 0, &nameValue) == FcResultMatch) {
			strncpy(data[count].name, (char*)nameValue.u.s, sizeof(data[count].name) - 1);
			data[count].name[sizeof(data[count].name) - 1] = '\0';
		}
		data[count].scale = 1;
		count += 1;
	}

	FcFontSetDestroy(font_set);
	FcPatternDestroy(pattern);
	FcConfigDestroy(config);

	*out_info       = data;
	*out_info_count = count;
}

#endif