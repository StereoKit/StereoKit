#include "platform_utils.h"
#include "android.h"

#include "../../stereokit.h"
#include "../../log.h"
#include "../../libraries/stref.h"

#include <stdio.h>
#include <stdlib.h>

#if WINDOWS_UWP
#include "uwp.h"

#include <winrt/Windows.UI.Popups.h>
#include <winrt/Windows.UI.Core.h> 
#include <winrt/Windows.ApplicationModel.Core.h>
#include <winrt/Windows.Foundation.Collections.h>
#endif

#if _WIN32
#include "win32.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

#if __ANDROID__
#include <unistd.h>
#include <android/log.h>
#include <android/asset_manager.h>
#include <errno.h>
#endif

namespace sk {

///////////////////////////////////////////

bool in_messagebox = false;
void platform_msgbox_err(const char *text, const char *header) {
#if WINDOWS_UWP
	char *src_text  = string_copy(text);
	char *src_title = string_copy(header);
	in_messagebox = true;
	winrt::Windows::ApplicationModel::Core::CoreApplication::MainView().CoreWindow().Dispatcher().RunAsync(
		winrt::Windows::UI::Core::CoreDispatcherPriority::Normal, [src_text,src_title]() {
		size_t   size_text  = strlen(src_text)+1;
		size_t   size_title = strlen(src_title)+1;
		wchar_t *w_text  = (wchar_t*)malloc(sizeof(wchar_t)*size_text);
		wchar_t *w_title = (wchar_t*)malloc(sizeof(wchar_t)*size_title);
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
		Sleep(100);
	}
#elif _MSC_VER
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
#if __ANDROID__
	// See: http://www.50ply.com/blog/2013/01/19/loading-compressed-android-assets-with-file-pointer/
	AAsset *asset = AAssetManager_open(android_asset_manager, filename, 0);
	FILE   *fp;
	if (asset) {
		fp = funopen(asset, 
			[](void *cookie, char *buf, int size) {return AAsset_read((AAsset *)cookie, buf, size); }, 
			[](void *cookie, const char *buf, int size) {return EACCES; }, 
			[](void *cookie, fpos_t offset, int whence) {return AAsset_seek((AAsset *)cookie, offset, whence); }, 
			[](void *cookie) { AAsset_close((AAsset *)cookie); return 0; } );
	} else {
		fp = fopen(filename, "rb");
	}

	if (fp == nullptr) {
		log_errf("Can't find file %s!", filename);
		return false;
	}
#elif __linux__

	FILE *fp;
	if (fopen(&fp, filename, "rb") != 0 || fp == nullptr) {
		log_errf("Can't find file %s!", filename);
		return false;
	}

#else
	FILE *fp;
	if (fopen_s(&fp, filename, "rb") != 0 || fp == nullptr) {
		log_errf("Can't find file %s!", filename);
		return false;
	}
#endif

	// Get length of file
	fseek(fp, 0L, SEEK_END);
	*out_size = ftell(fp);
	rewind(fp);

	// Read the data
	*out_data = malloc(*out_size+1);
	if (*out_data == nullptr) { *out_size = 0; fclose(fp); return false; }
	fread (*out_data, 1, *out_size, fp);
	fclose(fp);

#if __ANDROID__
	if (asset) AAsset_close(asset);
#endif

	// Stick an end string 0 character at the end in case the caller wants
	// to treat it like a string
	((uint8_t *)*out_data)[*out_size] = 0;

	return true;
}

///////////////////////////////////////////

bool platform_get_cursor(vec2 &out_pos) {
	bool result = false;
#if WINDOWS_UWP
	result = uwp_get_mouse(out_pos);
#elif _MSC_VER
	POINT cursor_pos;
	result =  GetCursorPos  (&cursor_pos)
		   && ScreenToClient((HWND)win32_hwnd(), &cursor_pos);
	out_pos.x = (float)cursor_pos.x;
	out_pos.y = (float)cursor_pos.y;
#else
#endif
	return result;
}

///////////////////////////////////////////

void platform_set_cursor(vec2 window_pos) {
#if WINDOWS_UWP
	uwp_set_mouse(window_pos);
#elif _MSC_VER
	POINT pt = { (LONG)window_pos.x, (LONG)window_pos.y };
	ClientToScreen((HWND)win32_hwnd(), &pt);
	SetCursorPos  (pt.x, pt.y);
#endif
}

///////////////////////////////////////////

float platform_get_scroll() {
#if WINDOWS_UWP
	return uwp_get_scroll();
#elif _WIN32
	return win32_scroll;
#else
	return 0;
#endif
}

///////////////////////////////////////////

bool platform_key_down(key_ key) {
#if WINDOWS_UWP
	return uwp_key_down(key);
#elif _WIN32
	return GetKeyState(key) & (key == key_caps_lock ? 0x1 : 0x8000);
#else
	return false;
#endif
}

///////////////////////////////////////////

void platform_debug_output(log_ level, const char *text) {
#if _WIN32
	OutputDebugStringA(text);
#else
	int32_t priority = ANDROID_LOG_INFO;
	if      (level == log_diagnostic) priority = ANDROID_LOG_VERBOSE;
	else if (level == log_inform    ) priority = ANDROID_LOG_INFO;
	else if (level == log_warning   ) priority = ANDROID_LOG_WARN;
	else if (level == log_error     ) priority = ANDROID_LOG_ERROR;
	__android_log_write(priority, "StereoKit", text); 
#endif
}

///////////////////////////////////////////

void platform_sleep(int ms) { 
#ifdef _WIN32
	Sleep(ms);
#else
	usleep(ms * 1000);
#endif
}

}