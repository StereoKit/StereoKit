#include "file_picker.h"
#include "../_stereokit.h"
#include "../stereokit_ui.h"
#include "../sk_memory.h"
#include "../libraries/array.h"
#include "../libraries/stref.h"
#include "../libraries/ferr_hash.h"
#include "../systems/platform/platform_utils.h"

#if defined(SK_OS_WINDOWS)
#include "../systems/platform/win32.h"
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <commdlg.h>
#elif defined(SK_OS_WINDOWS_UWP)

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
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
#endif

namespace sk {

///////////////////////////////////////////

struct fp_item_t {
	char *name;
	char  ext[32];
	bool  file;
};

#if defined(SK_OS_WINDOWS_UWP)
struct fp_file_cache_t {
public:
	uint64_t    name_hash;
	StorageFile file = nullptr;
};
std::vector<fp_file_cache_t> fp_file_cache    = {};
int32_t                      fp_file_cache_id = 0;
#endif

char                         fp_filename[1024];
bool                         fp_call          = false;
void                        *fp_call_data     = nullptr;
void                       (*fp_callback)(void *callback_data, bool32_t confirmed, const char *filename) = nullptr;

bool                         fp_show          = false;
file_filter_t               *fp_filters       = nullptr;
int32_t                      fp_filter_count  = 0;
char                        *fp_title         = nullptr;
char                        *fp_folder        = nullptr;
array_t<fp_item_t>           fp_items         = {};
pose_t                       fp_win_pose      = pose_identity;

///////////////////////////////////////////

void file_picker_open_folder(const char *folder);

///////////////////////////////////////////

void platform_file_picker(picker_mode_ mode, file_filter_t *filters, int32_t filter_count, void *callback_data, void (*on_confirm)(void *callback_data, bool32_t confirmed, const char *filename)) {
#if defined(SK_OS_WINDOWS)
	if (sk_active_display_mode() != display_mode_flatscreen) {
		fp_filename[0] = '\0';

		// Build a filter string
		char *filter = string_append(nullptr , 1, "(");
		for (int32_t e = 0; e < filter_count; e++) filter = string_append(filter, e==filter_count-1?1:2, filters[e].ext, ", ");
		filter = string_append(filter, 1, ")\1");
		for (int32_t e = 0; e < filter_count; e++) filter = string_append(filter, e==filter_count-1?1:2, filters[e].ext, ";");
		filter = string_append(filter, 1, "\1");
		int32_t len = strlen(filter);
		for (int32_t i = 0; i < len; i++) if (filter[i] == '\1') filter[i] = '\0'; 

		OPENFILENAMEA settings = {};
		settings.lStructSize  = sizeof(settings);
		settings.nMaxFile     = sizeof(fp_filename);
		settings.hwndOwner    = (HWND)win32_hwnd();
		settings.lpstrFile    = fp_filename;
		settings.lpstrFilter  = filter;
		settings.nFilterIndex = 1;
		settings.Flags        = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
		if (GetOpenFileName(&settings) == true) {
			if (on_confirm) on_confirm(callback_data, true, fp_filename);
		} else {
			if (on_confirm) on_confirm(callback_data, false, nullptr);
		}

		free(filter);
		return;
	}
#elif defined(SK_OS_WINDOWS_UWP)
	fp_callback  = on_confirm;
	fp_call_data = callback_data;

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
			
			fp_filename[0] = '\0';
			fp_file_cache_id += 1;
			snprintf(fp_filename, sizeof(fp_filename), "stereokit_cache_file:\\%d", fp_file_cache_id);
			WideCharToMultiByte(CP_UTF8, 0, result.Path().c_str(), result.Path().size()+1, fp_filename, sizeof(fp_filename), nullptr, nullptr);

			fp_file_cache_t item;
			item.file      = result;
			item.name_hash = hash_fnv64_string(fp_filename);
			fp_file_cache.push_back(item);
			fp_call = true;
		});
	});
	return;
#endif

	// Set up the fallback file picker

	// Make the title text for the window
	free(fp_title); 
	fp_title = nullptr;
	switch (mode) {
	case picker_mode_save:   fp_title = string_append(fp_title, 1, "Save"); break;
	case picker_mode_folder: fp_title = string_append(fp_title, 1, "Select Folder"); break;
	case picker_mode_open: {
		fp_title = string_append(fp_title, 1, "Open (");
		for (int32_t e = 0; e < filter_count; e++)
			fp_title = string_append(fp_title, e==filter_count-1?1:2, filters[e].ext, ", ");
		fp_title = string_append(fp_title, 1, ")");
	} break;
	}

	file_picker_open_folder(fp_folder);

	fp_show = true;
}

///////////////////////////////////////////

void file_picker_open_folder(const char *folder) {
	if (folder == nullptr) {
		folder = platform_working_dir();
	}

	char *new_folder = string_copy(folder);
	free(fp_folder);
	fp_folder = new_folder;
}

///////////////////////////////////////////

void file_picker_update() {
	if (fp_call) {
		if (fp_callback) fp_callback(fp_call_data, true, fp_filename);
		fp_callback  = nullptr;
		fp_call_data = nullptr;
		fp_call      = false;
	}

	if (fp_show) {
		ui_push_id("_skp");
		ui_window_begin(fp_title, fp_win_pose, { .4f,0 });

		if (ui_button("Up")) {}
		ui_sameline();
		ui_label_sz(fp_folder, {ui_area_remaining().x, ui_line_height()});

		if (fp_filename[0] != '\0') {
			if (ui_button("Open")) {}
			ui_sameline();
			ui_label(fp_filename);
		} else {
			ui_label("");
		}

		ui_hseparator();

		ui_window_end();
		ui_pop_id();
	}
}

///////////////////////////////////////////

void file_picker_shutdown() {
	free(fp_title ); fp_title  = nullptr;
	free(fp_folder); fp_folder = nullptr;
	fp_items.free();
}

///////////////////////////////////////////

bool file_picker_cache(const char *filename, void **out_data, size_t *out_size) {
#if defined(SK_OS_WINDOWS_UWP)
	uint64_t hash = hash_fnv64_string(filename);
	for (size_t i = 0; i < fp_file_cache.size(); i++) {
		if (fp_file_cache[i].name_hash == hash) {
			IRandomAccessStreamWithContentType stream = fp_file_cache[i].file.OpenReadAsync().get();
			Buffer buffer(stream.Size());
			winrt::Windows::Foundation::IAsyncOperationWithProgress<IBuffer, uint32_t> progress = stream.ReadAsync(buffer, stream.Size(), InputStreamOptions::None);
			IBuffer result = progress.get();

			*out_size = result.Length();
			*out_data = sk_malloc(*out_size + 1);
			memcpy(*out_data, result.data(), *out_size);

			stream.Close();
			fp_file_cache[i].file = nullptr;
			fp_file_cache.erase(fp_file_cache.begin() + i);
			i--;

			return true;
		}
	}
#endif
	return false;
}

///////////////////////////////////////////

}