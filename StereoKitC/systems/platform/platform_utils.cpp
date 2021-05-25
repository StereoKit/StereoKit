#define _CRT_SECURE_NO_WARNINGS

#include "platform_utils.h"
#include "android.h"

#include "../../stereokit.h"
#include "../../sk_memory.h"
#include "../../log.h"
#include "../../libraries/stref.h"
#include "../../libraries/ferr_hash.h"

#include <stdio.h>
#include <stdlib.h>

#ifdef SK_OS_WINDOWS_UWP
#include "uwp.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <winrt/Windows.UI.Popups.h>
#include <winrt/Windows.UI.Core.h>
#include <winrt/Windows.ApplicationModel.Core.h>
#include <winrt/Windows.Foundation.Collections.h>
#include <winrt/Windows.Storage.Pickers.h>
#include <winrt/Windows.Storage.Streams.h>

#include <vector>

using namespace winrt::Windows::UI::Core;
using namespace winrt::Windows::ApplicationModel::Core;
using namespace winrt::Windows::Storage;
using namespace winrt::Windows::Storage::Streams;

struct uwp_file_cache_t {
public:
	uint64_t    name_hash;
	StorageFile file = nullptr;
};
std::vector<uwp_file_cache_t> uwp_file_cache    = {};
int32_t                       uwp_file_cache_id = 0;

#endif

#ifdef SK_OS_WINDOWS
#include "win32.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <commdlg.h>
#endif

#ifdef SK_OS_ANDROID
#include <unistd.h>
#include <android/log.h>
#include <android/asset_manager.h>
#include <android/font_matcher.h>
#include <android/font.h>
#include <errno.h>
#endif

#ifdef SK_OS_LINUX
#include <unistd.h>
#include "linux.h"
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
	MessageBox(nullptr, text, header, MB_OK | MB_ICONERROR);
#else
	log_err("No messagebox capability for this platform!");
#endif
}

///////////////////////////////////////////

bool platform_read_file(const char *filename, void **out_data, size_t *out_size) {
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
	uint64_t hash = hash_fnv64_string(filename);
	for (size_t i = 0; i < uwp_file_cache.size(); i++) {
		if (uwp_file_cache[i].name_hash == hash) {
			IRandomAccessStreamWithContentType stream = uwp_file_cache[i].file.OpenReadAsync().get();
			Buffer buffer(stream.Size());
			winrt::Windows::Foundation::IAsyncOperationWithProgress<IBuffer, uint32_t> progress = stream.ReadAsync(buffer, stream.Size(), InputStreamOptions::None);
			IBuffer result = progress.get();

			*out_size = result.Length();
			*out_data = sk_malloc(*out_size + 1);
			memcpy(*out_data, result.data(), *out_size);

			stream.Close();
			uwp_file_cache[i].file = nullptr;
			uwp_file_cache.erase(uwp_file_cache.begin() + i);
			i--;

			return true;
		}
	}
#endif
	FILE *fp = fopen(filename, "rb");
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
	fread (*out_data, 1, *out_size, fp);
	fclose(fp);

	// Stick an end string 0 character at the end in case the caller wants
	// to treat it like a string
	((uint8_t *)*out_data)[*out_size] = 0;

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
#else
	(void)level;
	(void)text;
#endif
}

///////////////////////////////////////////

void platform_sleep(int ms) {
#if defined(SK_OS_WINDOWS) || defined(SK_OS_WINDOWS_UWP)
	Sleep(ms);
#elif defined(SK_OS_LINUX)
    sleep(ms / 1000);
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

void platform_keyboard_show(bool visible) {
#if defined(SK_OS_WINDOWS_UWP)
	uwp_show_keyboard(visible);
#else
#endif
}

///////////////////////////////////////////

bool platform_keyboard_visible() {
	return false;
}

///////////////////////////////////////////

void platform_default_font(char *fontname_buffer, size_t buffer_size) {
#if defined(SK_OS_ANDROID)

	// If we're using Android API 29+, we can just look up the system font!
	bool found_font = false;
#if __ANDROID_API__ >= 29
	AFontMatcher *matcher = AFontMatcher_create();
	uint16_t      text[2] = {'A', 0};
	AFont        *font    = AFontMatcher_match(matcher, "sans-serif", text, 2, nullptr);
	if (font) {
		const char *result = AFont_getFontFilePath(font);
		snprintf(fontname_buffer, buffer_size, result);
		AFont_close(font);
	}
	AFontMatcher_destroy(matcher);
#endif
	// We can fall back to a plausible default.
	if (!found_font)
		snprintf(fontname_buffer, buffer_size, "/system/fonts/DroidSans.ttf");

#elif defined(SK_OS_LINUX)
	snprintf(fontname_buffer, buffer_size, "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf");
#else
	snprintf(fontname_buffer, buffer_size, "C:\\Windows\\Fonts\\segoeui.ttf");
#endif
}

///////////////////////////////////////////

char   file_picker_filename[1024];
bool   file_picker_call      = false;
void  *file_picker_call_data = nullptr;
void (*file_picker_callback)(void *callback_data, bool32_t confirmed, const char *filename) = nullptr;

void platform_file_picker(picker_mode_ mode, file_filter_t *filters, int32_t filter_count, void *callback_data, void (*on_confirm)(void *callback_data, bool32_t confirmed, const char *filename)) {
#if defined(SK_OS_WINDOWS)
	if (sk_active_display_mode() == display_mode_flatscreen) {
		file_picker_filename[0] = '\0';

		// Build a filter string
		char filter[1024];
		filter[0] = '\0';
		snprintf(filter, sizeof(filter), "(");
		for (int32_t i = 0; i < filter_count; i++)
			snprintf(filter, sizeof(filter), "%s%s%s", filter, filters[i].ext, i == filter_count-1?"":", ");
		snprintf(filter, sizeof(filter), "%s)\1", filter);
		for (int32_t i = 0; i < filter_count; i++)
			snprintf(filter, sizeof(filter), "%s*%s%s", filter, filters[i].ext, i == filter_count-1?"":";");
		snprintf(filter, sizeof(filter), "%s\1", filter);
		int32_t len = strlen(filter);
		for (int32_t i = 0; i < len; i++) {
			if (filter[i] == '\1') filter[i] = '\0';
		}

		OPENFILENAMEA settings = {};
		settings.lStructSize  = sizeof(settings);
		settings.nMaxFile     = sizeof(file_picker_filename);
		settings.hwndOwner    = (HWND)win32_hwnd();
		settings.lpstrFile    = file_picker_filename;
		settings.lpstrFilter  = filter;
		settings.nFilterIndex = 1;
		settings.Flags        = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
		if (GetOpenFileName(&settings) == true) {
			if (on_confirm) on_confirm(callback_data, true, file_picker_filename);
		} else {
			if (on_confirm) on_confirm(callback_data, false, nullptr);
		}
	} else {
	}
#elif defined(SK_OS_WINDOWS_UWP)
	file_picker_callback  = on_confirm;
	file_picker_call_data = callback_data;

	Pickers::FileOpenPicker picker;
	for (int32_t i = 0; i < filter_count; i++) {
		wchar_t wext[32];
		MultiByteToWideChar(CP_UTF8, 0, filters[i].ext, strlen(filters[i].ext)+1, wext, 32);
		picker.FileTypeFilter().Append(wext);
	}
	CoreApplication::MainView().CoreWindow().Dispatcher().RunAsync(CoreDispatcherPriority::Normal, [picker, on_confirm, callback_data]() {
		picker.SuggestedStartLocation(Pickers::PickerLocationId::DocumentsLibrary);
		picker.PickSingleFileAsync().Completed([on_confirm, callback_data](auto &&result_info, auto && status) {
			auto result = result_info.get();
			
			file_picker_filename[0] = '\0';
			uwp_file_cache_id += 1;
			snprintf(file_picker_filename, sizeof(file_picker_filename), "stereokit_cache_file:\\%d", uwp_file_cache_id);
			WideCharToMultiByte(CP_UTF8, 0, result.Path().c_str(), result.Path().size()+1, file_picker_filename, sizeof(file_picker_filename), nullptr, nullptr);

			uwp_file_cache_t item;
			item.file      = result;
			item.name_hash = hash_fnv64_string(file_picker_filename);
			uwp_file_cache.push_back(item);
			file_picker_call = true;
		});
	});
#endif
}

///////////////////////////////////////////

bool platform_utils_init() {
	return true;
}

///////////////////////////////////////////

void platform_utils_update() {
	if (file_picker_call) {
		if (file_picker_callback) file_picker_callback(file_picker_call_data, true, file_picker_filename);
		file_picker_callback  = nullptr;
		file_picker_call_data = nullptr;
		file_picker_call      = false;
	}
}

///////////////////////////////////////////

void platform_utils_shutdown() {
}

}
