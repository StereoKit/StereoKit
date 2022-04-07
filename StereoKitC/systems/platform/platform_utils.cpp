#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif

#include "platform_utils.h"

#include "../../stereokit.h"
#include "../../sk_memory.h"
#include "../../sk_math.h"
#include "../../log.h"
#include "../../libraries/stref.h"
#include "../../libraries/array.h"
#include "../../tools/file_picker.h"
#include "../../tools/virtual_keyboard.h"
#include "../../systems/input_keyboard.h"
#include "../../asset_types/font.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#ifdef SK_OS_WINDOWS_UWP
#include "uwp.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <winrt/Windows.UI.Popups.h>
#include <winrt/Windows.UI.Core.h>
#include <winrt/Windows.ApplicationModel.Core.h>
#include <winrt/Windows.Foundation.Collections.h>
#endif

#ifdef SK_OS_WINDOWS
#include "win32.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

#ifdef SK_OS_ANDROID
#include "android.h"

#include <unistd.h>
#include <android/log.h>
#include <android/asset_manager.h>
#include <android/font_matcher.h>
#include <android/font.h>
#include <errno.h>
#endif

#ifdef SK_OS_LINUX
#include <unistd.h>
#include <dirent.h> 
#include "linux.h"
#include <fontconfig/fontconfig.h>
#endif

#ifdef SK_OS_WEB
#include "web.h"
#include <emscripten.h>
#include <emscripten/html5.h>
#include <stdio.h>
#endif

namespace sk {

///////////////////////////////////////////

bool in_messagebox = false;
void platform_msgbox_err(const char *text, const char *header) {
#if defined(SK_OS_WINDOWS_UWP)
	char *src_text  = string_copy(text);
	char *src_title = string_copy(header);
	in_messagebox = true;
	winrt::Windows::ApplicationModel::Core::CoreApplication::MainView().CoreWindow().Dispatcher().RunAsync(
		winrt::Windows::UI::Core::CoreDispatcherPriority::Normal, [src_text,src_title]() {
		size_t   size_text  = strlen(src_text)+1;
		size_t   size_title = strlen(src_title)+1;
		wchar_t *w_text  = sk_malloc_t(wchar_t, size_text);
		wchar_t *w_title = sk_malloc_t(wchar_t, size_title);
		mbstowcs_s(nullptr, w_text,  size_text,  src_text,   size_text);
		mbstowcs_s(nullptr, w_title, size_title, src_title, size_title);

		winrt::Windows::UI::Popups::MessageDialog dialog  = winrt::Windows::UI::Popups::MessageDialog(w_text, w_title);
		winrt::Windows::UI::Popups::UICommand     command = winrt::Windows::UI::Popups::UICommand{
			L"OK",
			winrt::Windows::UI::Popups::UICommandInvokedHandler{
				[w_text, w_title, src_text, src_title](winrt::Windows::UI::Popups::IUICommand const &) {
					in_messagebox = false;
					free(w_text);
					free(w_title);
					free(src_text);
					free(src_title);
					return;
				}
			}
		};
		dialog.Commands().Append(command);
		dialog.ShowAsync();
	});
	while (in_messagebox) {
		platform_sleep(100);
	}
#elif defined(SK_OS_WINDOWS)
	wchar_t *text_w   = platform_to_wchar(text);
	wchar_t *header_w = platform_to_wchar(header);
	MessageBoxW(nullptr, text_w, header_w, MB_OK | MB_ICONERROR);
	free(text_w);
	free(header_w);
#else
	log_err("No messagebox capability for this platform!");
#endif
}

///////////////////////////////////////////

bool32_t platform_read_file(const char *filename, void **out_data, size_t *out_size) {
	*out_data = nullptr;
	*out_size = 0;

	// Open file
#if defined(SK_OS_ANDROID)
	// See: http://www.50ply.com/blog/2013/01/19/loading-compressed-android-assets-with-file-pointer/

	// Try and load using Android API first!
	AAsset *asset = AAssetManager_open(android_asset_manager, filename, AASSET_MODE_BUFFER);
	if (asset) {
		*out_size = AAsset_getLength(asset);
		*out_data = sk_malloc(*out_size + 1);
		AAsset_read(asset, *out_data, *out_size);
		AAsset_close(asset);
		((uint8_t *)*out_data)[*out_size] = 0;
		return true;
	}

	// Fall back to standard file io functions, they -can- work on Android
#elif defined(SK_OS_WINDOWS_UWP)
	// See if we have a Handle cached from the FilePicker that matches this
	// file name.
	if (file_picker_cache_read(filename, out_data, out_size))
		return true;
#endif

#if defined(SK_OS_LINUX)
	// Linux thinks folders are files, but then fails in a bad way when
	// treating them like files.
	struct stat buffer;
	if (stat(filename, &buffer) == 0 && (S_ISDIR(buffer.st_mode))) {
		log_diagf("platform_read_file can't read folders: %s", filename);
		return false;
	}
#endif

#if defined(SK_OS_WINDOWS) || defined(SK_OS_WINDOWS_UWP)
	int32_t  wsize     = MultiByteToWideChar(CP_UTF8, 0, filename, -1, nullptr, 0);
	wchar_t *wfilename = sk_malloc_t(wchar_t, wsize);
	MultiByteToWideChar(CP_UTF8, 0, filename, -1, wfilename, wsize);
	FILE *fp = _wfopen(wfilename, L"rb");
	free(wfilename);
#else
	FILE *fp = fopen(filename, "rb");
#endif
	if (fp == nullptr) {
		log_diagf("platform_read_file can't find %s", filename);
		return false;
	}

	// Get length of file
	fseek(fp, 0, SEEK_END);
	*out_size = ftell(fp);
	fseek(fp, 0, SEEK_SET);

	// Read the data
	*out_data = sk_malloc(*out_size+1);
	size_t read = fread (*out_data, 1, *out_size, fp);
	fclose(fp);

	// Stick an end string 0 character at the end in case the caller wants
	// to treat it like a string
	((uint8_t *)*out_data)[*out_size] = 0;

	return read != 0 || *out_size == 0;
}

///////////////////////////////////////////

bool32_t platform_write_file(const char *filename, void *data, size_t size) {
#if defined(SK_OS_WINDOWS_UWP)
	// See if we have a Handle cached from the FilePicker that matches this
	// file name.
	if (file_picker_cache_save(filename, data, size))
		return true;
#endif

#if defined(SK_OS_WINDOWS) || defined(SK_OS_WINDOWS_UWP)
	int32_t  wsize     = MultiByteToWideChar(CP_UTF8, 0, filename, -1, nullptr, 0);
	wchar_t *wfilename = sk_malloc_t(wchar_t, wsize);
	MultiByteToWideChar(CP_UTF8, 0, filename, -1, wfilename, wsize);
	FILE *fp = _wfopen(wfilename, L"wb");
	free(wfilename);
#else
	FILE *fp = fopen(filename, "wb");
#endif
	if (fp == nullptr) {
		log_diagf("platform_read_file can't write %s", filename);
		return false;
	}

	fwrite(data, 1, size, fp);
	fclose(fp);

	return true;
}

///////////////////////////////////////////

bool platform_get_cursor(vec2 &out_pos) {
	bool result = false;
#if defined(SK_OS_WINDOWS_UWP)
	result = uwp_get_mouse(out_pos);
#elif defined(SK_OS_WINDOWS)
	POINT cursor_pos;
	result =  GetCursorPos  (&cursor_pos)
		   && ScreenToClient((HWND)win32_hwnd(), &cursor_pos);
	out_pos.x = (float)cursor_pos.x;
	out_pos.y = (float)cursor_pos.y;
#elif defined(SK_OS_LINUX)
	result = linux_get_cursor(out_pos);
#elif defined(SK_OS_WEB)
	result = web_get_cursor(out_pos);
#else
#endif
	return result;
}

///////////////////////////////////////////

void platform_set_cursor(vec2 window_pos) {
#if defined(SK_OS_WINDOWS_UWP)
	uwp_set_mouse(window_pos);
#elif defined(SK_OS_WINDOWS)
	POINT pt = { (LONG)window_pos.x, (LONG)window_pos.y };
	ClientToScreen((HWND)win32_hwnd(), &pt);
	SetCursorPos  (pt.x, pt.y);
#elif defined(SK_OS_LINUX)
	linux_set_cursor(window_pos);
#elif defined(SK_OS_WEB)
	web_set_cursor(window_pos);
#endif
}

///////////////////////////////////////////

float platform_get_scroll() {
#if defined(SK_OS_WINDOWS_UWP)
	return uwp_get_scroll();
#elif defined(SK_OS_WINDOWS)
	return win32_scroll;
#elif defined(SK_OS_LINUX)
	return linux_get_scroll();
#elif defined(SK_OS_WEB)
	return web_get_scroll();
#else
	return 0;
#endif
}

///////////////////////////////////////////

void platform_debug_output(log_ level, const char *text) {
#if defined(SK_OS_WINDOWS) || defined(SK_OS_WINDOWS_UWP)
	OutputDebugStringA(text);
	(void)level;
#elif defined(SK_OS_ANDROID)
	int32_t priority = ANDROID_LOG_INFO;
	if      (level == log_diagnostic) priority = ANDROID_LOG_VERBOSE;
	else if (level == log_inform    ) priority = ANDROID_LOG_INFO;
	else if (level == log_warning   ) priority = ANDROID_LOG_WARN;
	else if (level == log_error     ) priority = ANDROID_LOG_ERROR;
	__android_log_write(priority, "StereoKit", text);
#elif defined(SK_OS_WEB)
	if      (level == log_diagnostic) emscripten_console_log(text);
	else if (level == log_inform    ) emscripten_console_log(text);
	else if (level == log_warning   ) emscripten_console_warn(text);
	else if (level == log_error     ) emscripten_console_error(text);
#else
	(void)level;
	(void)text;
#endif
}

///////////////////////////////////////////

#if defined(SK_OS_ANDROID)
#include <unwind.h>
#include <dlfcn.h>
#include <cxxabi.h>

struct android_backtrace_state {
	void **current;
	void **end;
};

void platform_print_callstack() {
	const int max = 100;
	void* buffer[max];

	android_backtrace_state state;
	state.current = buffer;
	state.end = buffer + max;

	_Unwind_Backtrace([](struct _Unwind_Context* context, void* arg) {
		android_backtrace_state* state = (android_backtrace_state *)arg;
		uintptr_t pc = _Unwind_GetIP(context);
		if (pc) {
			if (state->current == state->end)
				return (_Unwind_Reason_Code)_URC_END_OF_STACK;
			else
				*state->current++ = reinterpret_cast<void*>(pc);
		}
		return (_Unwind_Reason_Code)_URC_NO_REASON;
	}, &state);

	int count = (int)(state.current - buffer);

	for (int idx = 0; idx < count; idx++)  {
		const void* addr   = buffer[idx];
		const char* symbol = "";

		Dl_info info;
		if (dladdr(addr, &info) && info.dli_sname)
			symbol = info.dli_sname;
		int   status    = 0; 
		char *demangled = __cxxabiv1::__cxa_demangle(symbol, 0, 0, &status); 

		sk::log_diagf("%03d: 0x%p %s", idx, addr,
			(nullptr != demangled && 0 == status) ?
			demangled : symbol);

		if (nullptr != demangled)
			free(demangled);
	}
}
#else
void platform_print_callstack() {
}
#endif

///////////////////////////////////////////

void platform_sleep(int ms) {
#if defined(SK_OS_WINDOWS) || defined(SK_OS_WINDOWS_UWP)
	Sleep(ms);
#elif defined(SK_OS_LINUX)
    sleep(ms / 1000);
#elif defined(SK_OS_WEB)
	emscripten_sleep(ms);
#else
	usleep(ms * 1000);
#endif
}

///////////////////////////////////////////

bool platform_keyboard_available() {
#if defined(SK_OS_WINDOWS_UWP)
	return true;
#else
	return false;
#endif
}

///////////////////////////////////////////

bool32_t force_fallback_keyboard = false;
bool32_t platform_keyboard_get_force_fallback() {
	return force_fallback_keyboard;
}

///////////////////////////////////////////

void platform_keyboard_set_force_fallback(bool32_t force_fallback) {
	force_fallback_keyboard = force_fallback;
}

///////////////////////////////////////////

void platform_keyboard_show(bool32_t visible, text_context_ type) {
#if defined(SK_OS_WINDOWS_UWP)
	// UWP handles soft keyboards on its own quite nicely!
	if (!force_fallback_keyboard) {
		uwp_show_keyboard(visible);
		return;
	}
#endif

	// Since we're now using the fallback keyboard, we need to balance soft
	// keyboards with physical keyboard on our own.
	// - It's always OK to set visible to false.
	// - Flatscreen never needs a soft keyboard.
	// - We'll assume someone in XR needs a soft keyboard up until they
	//   touch a physical keyboard. If a physical keyboard has been
	//   touched, then we'll avoid popping up a soft keyboard until a few
	//   minutes have passed since that interaction. TODO: this behavior
	//   may need some revision based on how people interact with it.
	const float physical_interact_timeout = 60 * 5; // 5 minutes
	if (visible == false) virtualkeyboard_open(false, type);
	else if (sk_active_display_mode() != display_mode_flatscreen &&
	         (input_last_physical_keypress < 0 || (time_getf()-input_last_physical_keypress) > physical_interact_timeout) ) {

		virtualkeyboard_open(visible, type);
	}
}

///////////////////////////////////////////

bool32_t platform_keyboard_visible() {
#if defined(SK_OS_WINDOWS_UWP)
	if (!force_fallback_keyboard)
		return uwp_keyboard_visible();
#endif
	return virtualkeyboard_get_open();
}

///////////////////////////////////////////

bool platform_file_exists(const char *filename) {
	struct stat buffer;
	return (stat (filename, &buffer) == 0);
}

///////////////////////////////////////////

font_t platform_default_font() {
#if defined(SK_OS_ANDROID)

	// If we're using Android API 29+, we can just look up the system font!
	array_t<const char *> fonts         = array_t<const char *>::make(2);
	font_t                result        = nullptr;
	const char           *file_latin    = nullptr;
	const char           *file_japanese = nullptr;

#if __ANDROID_API__ >= 29
	AFontMatcher *matcher = AFontMatcher_create();
	AFont *font_latin    = AFontMatcher_match(matcher, "sans-serif", u"A", 1, nullptr);
	AFont *font_japanese = AFontMatcher_match(matcher, "sans-serif", u"あ", 1, nullptr);
	if (font_latin   ) file_latin    = AFont_getFontFilePath(font_latin);
	if (font_japanese) file_japanese = AFont_getFontFilePath(font_japanese);
#endif
	if      (file_latin != nullptr)                                      fonts.add(file_latin);
	else if (platform_file_exists("/system/fonts/NotoSans-Regular.ttf")) fonts.add("/system/fonts/NotoSans-Regular.ttf");
	else if (platform_file_exists("/system/fonts/Roboto-Regular.ttf"  )) fonts.add("/system/fonts/Roboto-Regular.ttf");
	else if (platform_file_exists("/system/fonts/DroidSans.ttf"       )) fonts.add("/system/fonts/DroidSans.ttf");

	if      (file_japanese != nullptr)                                      fonts.add(file_japanese);
	else if (platform_file_exists("/system/fonts/NotoSansCJK-Regular.ttc")) fonts.add("/system/fonts/NotoSansCJK-Regular.ttc");
	else if (platform_file_exists("/system/fonts/DroidSansJapanese.ttf"  )) fonts.add("/system/fonts/DroidSansJapanese.ttf");

	if (fonts.count > 0)
		result = font_create_files(fonts.data, (int32_t)fonts.count);

#if __ANDROID_API__ >= 29
	if (font_latin   ) AFont_close(font_latin);
	if (font_japanese) AFont_close(font_japanese);
	AFontMatcher_destroy(matcher);
#endif
	fonts.free();
	return result;

#elif defined(SK_OS_LINUX)
	array_t<char *> fonts  = array_t<char *>::make(2);
	font_t          result = nullptr;

	FcConfig  *config  = FcInitLoadConfigAndFonts();
	FcPattern *pattern = FcNameParse((const FcChar8*)("sans-serif"));
	FcConfigSubstitute (config, pattern, FcMatchPattern);
	FcDefaultSubstitute(pattern);

	FcResult   fc_result;
	FcPattern* font = FcFontMatch(config, pattern, &fc_result);
	if (font) {
		FcChar8* file = nullptr;
		if (FcPatternGetString(font, FC_FILE, 0, &file) == FcResultMatch) {
			// Put the font file path in the proper string
			fonts.add(string_copy((char*)file));
		}
		FcPatternDestroy(font);
	}
	FcPatternDestroy(pattern);
	FcConfigDestroy (config);

	result = font_create_files((const char **)fonts.data, (int32_t)fonts.count);
	fonts.each(free);
	fonts.free();
	return result;
#elif defined(SK_OS_WEB)
	return font_create_default();
#else
	array_t<const char *> fonts = array_t<const char *>::make(3);
	fonts.add(platform_file_exists("C:/Windows/Fonts/segoeui.ttf")
		? "C:/Windows/Fonts/segoeui.ttf"
		: "C:/Windows/Fonts/arial.ttf");

	if      (platform_file_exists("C:/Windows/Fonts/YuGothR.ttc")) fonts.add("C:/Windows/Fonts/YuGothR.ttc");
	else if (platform_file_exists("C:/Windows/Fonts/YuGothm.ttc")) fonts.add("C:/Windows/Fonts/YuGothm.ttc");
	else if (platform_file_exists("C:/Windows/Fonts/Meiryo.ttc" )) fonts.add("C:/Windows/Fonts/Meiryo.ttc");
	else if (platform_file_exists("C:/Windows/Fonts/Yumin.ttf"  )) fonts.add("C:/Windows/Fonts/Yumin.ttf");

	if (platform_file_exists("C:/Windows/Fonts/segmdl2.ttf")) fonts.add("C:/Windows/Fonts/segmdl2.ttf");

	font_t result = font_create_files(fonts.data, (int32_t)fonts.count);
	fonts.free();
	return result;
#endif
}

///////////////////////////////////////////

char *platform_working_dir() {
#if defined(SK_OS_WINDOWS) || defined(SK_OS_WINDOWS_UWP)
	int32_t len    = GetCurrentDirectoryA(0, nullptr);
	char   *result = sk_malloc_t(char, len);
	GetCurrentDirectoryA(len, result);
#elif defined(SK_OS_WEB)
	char *result = string_copy("/");
#else
	int32_t len    = 260;
	char   *result = sk_malloc_t(char, len);
	result[0] = '\0';
	while (len < 5000 && getcwd(result, len) == nullptr) {
		free(result);
		len       = len * 2;
		result    = sk_malloc_t(char, len);
		result[0] = '\0';
	}
#endif
	return result;
}

///////////////////////////////////////////

void  platform_iterate_dir(const char *directory_path, void *callback_data, void (*on_item)(void *callback_data, const char *name, bool file)) {
#if defined(SK_OS_WINDOWS)
	if (string_eq(directory_path, "")) {
		char drive_names[256];
		GetLogicalDriveStrings(sizeof(drive_names), drive_names);
		char *curr = drive_names;
		while (*curr != '\0') {
			on_item(callback_data, curr, false);
			curr = curr + strlen(curr)+1;
		}
		return;
	}

	WIN32_FIND_DATA info;
	HANDLE          handle = nullptr;

	char *filter = string_copy(directory_path);
	if (string_endswith(filter, "\\"))
		filter = string_append(filter, 1, "*.*");
	else filter = string_append(filter, 1, "\\*.*");

	handle = FindFirstFile(filter, &info);
	if (handle == INVALID_HANDLE_VALUE) return;

	while (handle) {
		if (!string_eq(info.cFileName, ".") && !string_eq(info.cFileName, "..")) {
			if (info.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
				on_item(callback_data, info.cFileName, false);
			else
				on_item(callback_data, info.cFileName, true);
		}

		if (!FindNextFile(handle, &info)) {
			FindClose(handle);
			handle = nullptr;
		}
	}
#elif defined(SK_OS_LINUX)
	if (string_eq(directory_path, "")) {
		directory_path = platform_path_separator;
	}

	DIR           *dir;
	struct dirent *dir_info;
	dir = opendir(directory_path);
	if (dir) {
		while ((dir_info = readdir(dir)) != nullptr) {
			if (string_eq(dir_info->d_name, ".") || string_eq(dir_info->d_name, "..")) continue;

			if (dir_info->d_type == DT_DIR)
				on_item(callback_data, dir_info->d_name, false);
			else if (dir_info->d_type == DT_REG)
				on_item(callback_data, dir_info->d_name, true);
		}
		closedir(dir);
	}
#elif defined(SK_OS_WINDOWS_UWP)
	(void)on_item;
	(void)callback_data;
	(void)directory_path;
	log_err("platform_iterate_dir is not valid for UWP.");
#endif
}

///////////////////////////////////////////

char *platform_push_path_ref(char *path, const char *directory) {
	char *result = platform_push_path_new(path, directory);
	free(path);
	return result;
}

///////////////////////////////////////////

char *platform_pop_path_ref(char *path) {
	char *result = platform_pop_path_new(path);
	free(path);
	return result;
}

///////////////////////////////////////////

char *platform_push_path_new(const char *path, const char *directory) {
	if (path == nullptr || directory == nullptr) return nullptr;
	if (directory[0] == '\0') return string_copy(path);

	size_t len = strlen(path);
	if (len == 0) {
		bool trailing_cslash = string_endswith(directory, ":\\") || string_endswith(directory, ":/");
		bool trailing_c      = string_endswith(directory, ":");
		if      (trailing_cslash) return string_copy  (directory);
		else if (trailing_c)      return string_append(nullptr, 2, directory, platform_path_separator);
		else return nullptr;
	}

	bool ends_with   = path     [len-1] == '\\' || path     [len-1] == '/';
	bool starts_with = directory[0]     == '\\' || directory[0]     == '/';

	char *result = nullptr;
	if (!ends_with && !starts_with)
		result = string_append(nullptr, 3, path, platform_path_separator, directory);
	else if (ends_with && starts_with)
		result = string_append(nullptr, 2, path, directory[1]);
	else 
		result = string_append(nullptr, 2, path, directory);

	return result;
}

///////////////////////////////////////////

char *platform_pop_path_new(const char *path) {
	stref_t src_path  = stref_make(path);
	// Trim off trailing slashes
	while (src_path.length > 0 && (path[src_path.length-1] == '\\' || path[src_path.length-1] == '/'))
		src_path.length -= 1;

	int32_t last = maxi( stref_lastof(src_path, '\\'), stref_lastof(src_path, '/') );
	if (last != -1) {
		src_path = stref_substr(src_path, 0, last);
	} else {
#if defined(SK_OS_WINDOWS) || defined(SK_OS_WINDOWS_UWP)
		return string_copy("");
#else
		return string_copy(platform_path_separator);
#endif
	}

	last = maxi( stref_lastof(src_path, '\\'), stref_lastof(src_path, '/') );
	if (last == -1) {
		return string_append(stref_copy(src_path), 1, platform_path_separator);
	} else {
		return stref_copy(src_path);
	}
}

///////////////////////////////////////////

#if defined(SK_OS_WINDOWS) || defined(SK_OS_WINDOWS_UWP)
wchar_t *platform_to_wchar(const char *utf8_string) {
	int32_t  wsize  = MultiByteToWideChar(CP_UTF8, 0, utf8_string, -1, nullptr, 0);
	wchar_t *result = sk_malloc_t(wchar_t, wsize);
	MultiByteToWideChar(CP_UTF8, 0, utf8_string, -1, result, wsize);
	return result;
}

char *platform_from_wchar(const wchar_t *string) {
	int32_t len  = (int)(wcslen(string)+1);
	int32_t size = WideCharToMultiByte(CP_UTF8, 0, string, len, nullptr, 0, nullptr, nullptr);
	char *result = sk_malloc_t(char, size);
	WideCharToMultiByte               (CP_UTF8, 0, string, len, result, size, nullptr, nullptr);
	return result;
}
#endif

///////////////////////////////////////////

bool platform_utils_init() {
	virtualkeyboard_initialize();
	return true;
}

///////////////////////////////////////////

void platform_utils_update() {
	file_picker_update();
	virtualkeyboard_update();
}

///////////////////////////////////////////

void platform_utils_shutdown() {
	file_picker_shutdown();
}

}
