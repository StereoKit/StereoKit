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
	#include <stdio.h>

#elif defined(SK_OS_WINDOWS_UWP)

	#define WIN32_LEAN_AND_MEAN
	#include <windows.h>
	#include <winrt/Windows.UI.Core.h>
	#include <winrt/Windows.ApplicationModel.Core.h>
	#include <winrt/Windows.Foundation.Collections.h>
	#include <winrt/Windows.Storage.Pickers.h>
	#include <winrt/Windows.Storage.Streams.h>

	using namespace winrt::Windows::UI::Core;
	using namespace winrt::Windows::ApplicationModel::Core;
	using namespace winrt::Windows::Storage;
	using namespace winrt::Windows::Foundation;
	using namespace winrt::Windows::Storage::Streams;

#elif defined(SK_OS_WEB)

	#include <stdio.h>

#endif

namespace sk {

///////////////////////////////////////////

struct fp_item_t {
	char *name;
	bool  file;
};

#if defined(SK_OS_WINDOWS_UWP)
struct fp_file_cache_t {
public:
	uint64_t    name_hash;
	StorageFile file = nullptr;
};
std::vector<fp_file_cache_t> fp_file_cache;
#endif

char                         fp_filename [1024];
wchar_t                      fp_wfilename[1024];
char                         fp_buffer[1023];
bool                         fp_call          = false;
void                        *fp_call_data     = nullptr;
bool                         fp_call_status   = false;
void                       (*fp_callback)(void *callback_data, bool32_t confirmed, const char *filename, int32_t filename_length) = nullptr;

bool                         fp_show          = false;
picker_mode_                 fp_mode;
file_filter_t               *fp_filters       = nullptr;
int32_t                      fp_filter_count  = 0;
char                        *fp_title         = nullptr;
char                        *fp_folder        = nullptr;
char                        *fp_active        = nullptr;
array_t<fp_item_t>           fp_items         = {};
pose_t                       fp_win_pose      = pose_identity;

///////////////////////////////////////////

void file_picker_finish     ();
void file_picker_open_folder(const char *folder);
#if defined(SK_OS_WINDOWS_UWP)
void file_picker_uwp_picked (IAsyncOperation<StorageFile> result, AsyncStatus status);
#endif

///////////////////////////////////////////

void platform_file_picker(picker_mode_ mode, void *callback_data, void (*on_confirm)(void *callback_data, bool32_t confirmed, const char *filename), const file_filter_t *filters, int32_t filter_count) {
	// This is basically a manual lambda capture for the wrapper callback
	struct callback_t {
		void *callback_data;
		void (*on_confirm)(void *callback_data, bool32_t confirmed, const char *filename);
	};
	callback_t *data = sk_malloc_t(callback_t, 1);
	data->callback_data = callback_data;
	data->on_confirm    = on_confirm;

	// Call the file picker that does all the real work, and pass the callback
	// along to _our_ callback.
	platform_file_picker_sz(mode, data, [](void *callback_data, bool32_t confirmed, const char *filename, int32_t) {
		callback_t *data = (callback_t *)callback_data;
		if (data->on_confirm)
			data->on_confirm(data->callback_data, confirmed, filename);
		free(data);
	}, filters, filter_count);
}

///////////////////////////////////////////

char *platform_append_filter(char *to, const file_filter_t *filter, bool search_pattern, const char *postfix) {
	const char *curr = filter->ext;
	while (*curr == '.' || *curr == '*') curr++;

	return search_pattern
		? string_append(to, 3, "*.", curr, postfix)
		: string_append(to, 2, curr, postfix);
}

///////////////////////////////////////////

void platform_file_picker_sz(picker_mode_ mode, void *callback_data, void (*on_confirm)(void *callback_data, bool32_t confirmed, const char *filename, int32_t filename_length), const file_filter_t *filters, int32_t filter_count) {
#if defined(SK_OS_WINDOWS)
	if (sk_active_display_mode() == display_mode_flatscreen) {
		fp_wfilename[0] = '\0';

		// Build a filter string
		char *filter = string_append(nullptr , 1, "(");
		for (int32_t e = 0; e < filter_count; e++) filter = platform_append_filter(filter, &filters[e], false, e == filter_count - 1 ? "" : ", ");
		filter = string_append(filter, 1, ")\1");
		for (int32_t e = 0; e < filter_count; e++) filter = platform_append_filter(filter, &filters[e], true, e == filter_count - 1 ? "" : ";");
		filter = string_append(filter, 1, "\1Any (*.*)\1*.*\1");
		size_t len = strlen(filter);
		wchar_t *w_filter = platform_to_wchar(filter);
		for (size_t i = 0; i < len; i++) if (w_filter[i] == '\1') w_filter[i] = '\0'; 

		OPENFILENAMEW settings = {};
		settings.lStructSize  = sizeof(settings);
		settings.nMaxFile     = sizeof(fp_filename);
		settings.hwndOwner    = (HWND)win32_hwnd();
		settings.lpstrFile    = fp_wfilename;
		settings.lpstrFilter  = w_filter;
		settings.nFilterIndex = 1;

		if (mode == picker_mode_open) {
			settings.Flags      = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;
			settings.lpstrTitle = L"Open";
			if (GetOpenFileNameW(&settings) == TRUE) {
				char *filename = platform_from_wchar(fp_wfilename);
				if (on_confirm) on_confirm(callback_data, true, filename, (int32_t)(strlen(filename)+1));
				free(filename);
			} else {
				if (on_confirm) on_confirm(callback_data, false, nullptr, 0);
			}
		} else if (mode == picker_mode_save) {
			settings.Flags = OFN_PATHMUSTEXIST | OFN_NOCHANGEDIR;
			settings.lpstrTitle = L"Save As";
			if (GetSaveFileNameW(&settings) == TRUE) {
				char *filename = platform_from_wchar(fp_wfilename);
				if (on_confirm) on_confirm(callback_data, true, filename, (int32_t)(strlen(filename)+1));
				free(filename);
			} else {
				if (on_confirm) on_confirm(callback_data, false, nullptr, 0);
			}
		}

		free(w_filter);
		free(filter);
		return;
	}
#elif defined(SK_OS_WINDOWS_UWP)
	fp_callback  = on_confirm;
	fp_call_data = callback_data;

	CoreDispatcher dispatcher = CoreApplication::MainView().CoreWindow().Dispatcher();
	wchar_t        wext[32];
	if (mode == picker_mode_open) {
		Pickers::FileOpenPicker picker;
		for (int32_t i = 0; i < filter_count; i++) {
			const char *ext = filters[i].ext;
			while (*ext == '*') ext++;

			char *ext_mem = nullptr;
			if (*ext != '.') {
				ext_mem = string_append(nullptr, 2, ".", ext);
				ext = ext_mem;
			}

			MultiByteToWideChar(CP_UTF8, 0, ext, (int)strlen(ext)+1, wext, 32);
			picker.FileTypeFilter().Append(wext);

			free(ext_mem);
		}
		picker.SuggestedStartLocation(Pickers::PickerLocationId::DocumentsLibrary);
		dispatcher.RunAsync(CoreDispatcherPriority::Normal, [picker]() {
			picker.PickSingleFileAsync().Completed(file_picker_uwp_picked);
		});
	} else if (mode == picker_mode_save) {
		Pickers::FileSavePicker picker;
		auto exts{ winrt::single_threaded_vector<winrt::hstring>() };
		for (int32_t i = 0; i < filter_count; i++) {
			MultiByteToWideChar(CP_UTF8, 0, filters[i].ext, (int)strlen(filters[i].ext)+1, wext, 32);
			exts.Append(wext);
		}
		picker.FileTypeChoices().Insert(L"File Type", exts);
		picker.SuggestedStartLocation(Pickers::PickerLocationId::DocumentsLibrary);
		dispatcher.RunAsync(CoreDispatcherPriority::Normal, [picker]() {
			picker.PickSaveFileAsync().Completed(file_picker_uwp_picked);
		});
	}
	return;
#endif

#if !defined(SK_OS_WINDOWS_UWP)
	// Set up the fallback file picker

	// Make the title text for the window
	free(fp_title); 
	fp_title = nullptr;
	switch (mode) {
	case picker_mode_save:   fp_title = string_append(fp_title, 1, "Save As"); break;
	case picker_mode_open: {
		fp_title = string_append(fp_title, 1, "Open (");
		for (int32_t e = 0; e < filter_count; e++)
			fp_title = platform_append_filter(fp_title, &filters[e], false, e == filter_count - 1 ? "" : ", ");
		fp_title = string_append(fp_title, 1, ")");
	} break;
	}

	fp_filter_count = filter_count;
	free(fp_filters);
	fp_filters = sk_malloc_t(file_filter_t, fp_filter_count);
	memcpy(fp_filters, filters, sizeof(file_filter_t) * fp_filter_count);

	file_picker_open_folder(fp_folder);

	const pose_t *head = input_head();
	vec3          pos  = head->position + head->orientation*vec3_forward*.5f + head->orientation*vec3_up*0.2f;
	fp_win_pose = { pos, quat_lookat(pos, head->position) };

	fp_call_data = callback_data;
	fp_callback  = on_confirm;
	fp_mode = mode;
	fp_show = true;
#endif
}

///////////////////////////////////////////

void platform_file_picker_close() {
	if (fp_show) {
		file_picker_finish();
	}
}

///////////////////////////////////////////

bool32_t platform_file_picker_visible() {
	return fp_show;
}

///////////////////////////////////////////

#if defined(SK_OS_WINDOWS_UWP)
void file_picker_uwp_picked(IAsyncOperation<StorageFile> result, AsyncStatus status) {
	StorageFile file = result.get();

	if (status == AsyncStatus::Completed && file != nullptr) {
		WideCharToMultiByte(CP_UTF8, 0, file.Path().c_str(), file.Path().size()+1, fp_filename, sizeof(fp_filename), nullptr, nullptr);

		fp_file_cache_t item;
		item.file      = file;
		item.name_hash = hash_fnv64_string(fp_filename);
		fp_file_cache.push_back(item);
		fp_call        = true;
		fp_call_status = true;
	} else {
		fp_call        = true;
		fp_call_status = false;
	}
}
#endif

///////////////////////////////////////////

void file_picker_open_folder(const char *folder) {
	char *dir_mem = nullptr;
	if (folder == nullptr) {
		dir_mem = platform_working_dir();
		folder  = dir_mem;
	}

	fp_items.each([](fp_item_t &item) { free(item.name); });
	fp_items.clear();
	platform_iterate_dir(folder, nullptr, [](void*, const char *name, bool file) {
		bool valid = fp_filter_count == 0;
		// If the extention matches our filter, add it
		if (file) {
			for (int32_t e = 0; e < fp_filter_count; e++) {
				if (string_endswith(name, fp_filters[e].ext, false)) {
					valid = true;
					break;
				}
			}
		} else valid = true;

		if (valid) {
			fp_item_t item;
			item.file = file;
			item.name = string_copy(name);
			fp_items.add(item);
		}
	});
	fp_items.sort([](const fp_item_t &a, const fp_item_t &b) { return a.file != b.file ? a.file-b.file : strcmp(a.name, b.name); });

	char *new_folder = string_copy(folder);
	free(fp_folder);
	free(dir_mem);
	fp_folder = new_folder;
	fp_active = nullptr;
}

///////////////////////////////////////////

void file_picker_finish() {
	if (fp_callback) fp_callback(fp_call_data, fp_call_status, fp_filename, (int32_t)(strlen(fp_filename)+1));
	fp_call_status = false;
	fp_callback    = nullptr;
	fp_call_data   = nullptr;
	fp_call        = false;
	fp_show        = false;
}

///////////////////////////////////////////

void file_picker_update() {
	if (fp_show) {
		ui_push_id("_skp");
		ui_window_begin(fp_title, fp_win_pose, { .4f,0 });

		// Show the current directory
		if (ui_button("Up")) {
			char *path = platform_pop_path_new(fp_folder);
			file_picker_open_folder(path);
			free(path);
		}
		ui_sameline();
		ui_label_sz(fp_folder, {ui_layout_remaining().x, ui_line_height()});

		// Show the active item
		switch (fp_mode) {
		case picker_mode_save: {
			if (ui_button("Cancel")) {
				fp_call        = true;
				fp_call_status = false;
			}
			ui_sameline();
			if (ui_button("Save")) { 
				snprintf(fp_filename, sizeof(fp_filename), "%s%c%s", fp_folder, platform_path_separator_c, fp_buffer);
				fp_call        = true;
				fp_call_status = true;
			}
			ui_sameline();
			ui_input("SaveFile", fp_buffer, sizeof(fp_buffer), vec2{ ui_layout_remaining().x, ui_line_height() });
		} break;
		case picker_mode_open: {
			if (ui_button("Cancel")) { fp_call = true; fp_call_status = false; }
			ui_sameline();
			if (fp_active) {
				if (ui_button("Open")) { snprintf(fp_filename, sizeof(fp_filename), "%s%c%s", fp_folder, platform_path_separator_c, fp_active); fp_call = true; fp_call_status = true; }
				ui_sameline();
				ui_label(fp_active);
			} else {
				ui_label("");
			}
		} break;
		}

		ui_hseparator();

		// List the files
		vec2 size = { .085f, ui_line_height() * 1.5f };
		for (size_t i = 0; i < fp_items.count; i++) {
			if (ui_button_sz(fp_items[i].name, size)) {
				if (fp_items[i].file)
					fp_active = fp_items[i].name;
				else {
					char *path = platform_push_path_new(fp_folder, fp_items[i].name);
					file_picker_open_folder(path);
					free(path);
				}
			}
			ui_sameline();
		}

		ui_window_end();
		ui_pop_id();
	}

	if (fp_call) {
		file_picker_finish();
	}
}

///////////////////////////////////////////

void file_picker_shutdown() {
	free(fp_title ); fp_title  = nullptr;
	free(fp_folder); fp_folder = nullptr;
	fp_items.free();

#if defined(SK_OS_WINDOWS_UWP)
	//fp_file_cache.free();
#endif
}

///////////////////////////////////////////

bool file_picker_cache_read(const char *filename, void **out_data, size_t *out_size) {
#if defined(SK_OS_WINDOWS_UWP)
	uint64_t hash = hash_fnv64_string(filename);
	for (size_t i = 0; i < fp_file_cache.size(); i++) {
		if (fp_file_cache[i].name_hash == hash) {
			IRandomAccessStreamWithContentType stream = fp_file_cache[i].file.OpenReadAsync().get();
			Buffer buffer((uint32_t)stream.Size());
			winrt::Windows::Foundation::IAsyncOperationWithProgress<IBuffer, uint32_t> progress = stream.ReadAsync(buffer, (uint32_t)stream.Size(), InputStreamOptions::None);
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

bool file_picker_cache_save(const char *filename, void *data, size_t size) {
#if defined(SK_OS_WINDOWS_UWP)
	uint64_t hash = hash_fnv64_string(filename);
	for (size_t i = 0; i < fp_file_cache.size(); i++) {
		if (fp_file_cache[i].name_hash == hash) {
			winrt::array_view<uint8_t const> view{ (uint8_t *)data, (uint8_t *)data + size };
			FileIO::WriteBytesAsync(fp_file_cache[i].file, view);

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