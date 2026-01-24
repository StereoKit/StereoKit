/* SPDX-License-Identifier: MIT */
/* The authors below grant copyright rights under the MIT license:
 * Copyright (c) 2019-2023 Nick Klingensmith
 * Copyright (c) 2023 Qualcomm Technologies, Inc.
 */

#include "file_picker.h"

#include <sk_app.h>

#include "../stereokit_ui.h"
#include "../sk_memory.h"
#include "../sk_math.h"
#include "../libraries/array.h"
#include "../libraries/stref.h"
#include "../platforms/platform.h"

#include <stdio.h>

namespace sk {

///////////////////////////////////////////

struct fp_item_t {
	char    *name;
	bool     is_file;
	int64_t  size;
};

struct fp_path_t {
	char          *folder;
	array_t<char*> fragments;
};

enum fp_sort_by_ {
	fp_sort_by_name,
	fp_sort_by_size
};

char                         fp_filename [1024];
wchar_t                      fp_wfilename[1024];
char                         fp_buffer   [1023];
bool                         fp_call                  = false;
void                        *fp_call_data             = nullptr;
bool                         fp_call_status           = false;
bool                         fp_files_sorted_asc      = true;
fp_sort_by_                  fp_sortby                = fp_sort_by_name;
bool                         fp_sort_order_changed    = false;
bool                         fp_sort_order_asc        = true;
bool                         fp_list_mode             = false;
void                         (*fp_callback)(void *callback_data, bool32_t confirmed, const char *filename, int32_t filename_length) = nullptr;

bool                         fp_show                  = false;
picker_mode_                 fp_mode;
file_filter_t               *fp_filters               = nullptr;
int32_t                      fp_filter_count          = 0;
char                        *fp_title                 = nullptr;
char                        *fp_active                = nullptr;
array_t<fp_item_t>           fp_items                 = {};
pose_t                       fp_win_pose              = pose_identity;
fp_path_t                    fp_path                  = {};
int32_t                      fp_scroll_offset         = 0;
float                        fp_scroll_val            = 0;
sprite_t                     spr_folder               = nullptr;
sprite_t                     spr_up                   = nullptr;
sprite_t                     spr_down                 = nullptr;
sprite_t                     spr_grid                 = nullptr;
sprite_t                     spr_list                 = nullptr;

// State for pending native file dialog (used in flatscreen mode)
ska_file_dialog_id_t         fp_native_dialog_id      = 0;
void                        *fp_native_callback_data  = nullptr;
void                       (*fp_native_callback)(void *callback_data, bool32_t confirmed, const char* filename_ptr, int32_t filename_length) = nullptr;

///////////////////////////////////////////

void file_picker_finish     ();
void file_picker_open_folder(const char *folder);

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
		sk_free(data);
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

void platform_file_picker_sz(picker_mode_ mode, void *callback_data, void (*picker_callback_sz)(void *callback_data, bool32_t confirmed, const char* filename_ptr, int32_t filename_length), const file_filter_t* in_arr_filters, int32_t filter_count) {
	// Use native file dialog if available via sk_app
	// This works on desktop flatscreen, and some Android XR systems with native pickers
	ska_file_dialog_ dialog_type = (mode == picker_mode_open) ? ska_file_dialog_open : ska_file_dialog_save;
	if (ska_file_dialog_available(dialog_type)) {
		// Build ska_file_filter_t array from file_filter_t
		// SK's filters are individual extensions, sk_app uses space-separated extensions
		char* exts_str = nullptr;
		for (int32_t e = 0; e < filter_count; e++) {
			const char* ext = in_arr_filters[e].ext;
			while (*ext == '.' || *ext == '*') ext++; // Skip leading . or *
			exts_str = string_append(exts_str, 3, "*.", ext, e == filter_count - 1 ? "" : " ");
		}

		ska_file_filter_t ska_filter = {};
		ska_filter.name = "Files";
		ska_filter.mime = "*/*";
		ska_filter.exts = exts_str ? exts_str : "*.*";

		ska_file_dialog_request_t request = {};
		request.type          = dialog_type;
		request.title         = (mode == picker_mode_open) ? "Open" : "Save As";
		request.default_name  = nullptr;
		request.filters       = filter_count > 0 ? &ska_filter : nullptr;
		request.filter_count  = filter_count > 0 ? 1 : 0;
		request.allow_multiple = false;

		// Store callback info for when the event arrives
		fp_native_callback_data = callback_data;
		fp_native_callback      = picker_callback_sz;
		fp_native_dialog_id     = ska_file_dialog_show(&request);

		sk_free(exts_str);
		return;
	}

	// Set up the fallback file picker

	// Make the title text for the window
	sk_free(fp_title); 
	fp_title = nullptr;
	switch (mode) {
	case picker_mode_save:   fp_title = string_append(fp_title, 1, "Save As"); break;
	case picker_mode_open: {
		fp_title = string_append(fp_title, 1, "Open (");
		for (int32_t e = 0; e < filter_count; e++)
			fp_title = platform_append_filter(fp_title, &in_arr_filters[e], false, e == filter_count - 1 ? "" : ", ");
		fp_title = string_append(fp_title, 1, ")");
	} break;
	}
	fp_filter_count = filter_count;
	sk_free(fp_filters);
	fp_filters = sk_malloc_t(file_filter_t, fp_filter_count);
	memcpy(fp_filters, in_arr_filters, sizeof(file_filter_t) * fp_filter_count);

	file_picker_open_folder(fp_path.folder);

	fp_win_pose  = matrix_transform_pose( matrix_invert(render_get_cam_root()), ui_popup_pose({0,-0.1f,0}));
	fp_call_data = callback_data;
	fp_callback  = picker_callback_sz;
	fp_mode      = mode;
	fp_show      = true;

	if (spr_folder == nullptr) {
		spr_folder = sprite_find(ui_default_id_spr_toggle_off);
		spr_up     = sprite_find(ui_default_id_spr_arrow_up);
		spr_down   = sprite_find(ui_default_id_spr_arrow_down);
		spr_grid   = sprite_find(ui_default_id_spr_grid);
		spr_list   = sprite_find(ui_default_id_spr_list);
	}
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

void file_picker_open_folder(const char *folder) {
	char cwd_buf[1024];
	if (folder == nullptr) {
		ska_get_cwd(cwd_buf, sizeof(cwd_buf));
		folder = cwd_buf;
	}

	fp_items.each([](fp_item_t &item) { sk_free(item.name); });
	fp_items.clear();

	ska_dir_iterate(folder, nullptr, [](void*, const ska_dir_entry_t* entry) {
		bool is_file = !entry->is_dir;
		bool valid = fp_filter_count == 0;
		// If the extension matches our filter, add it
		if (is_file) {
			for (int32_t e = 0; e < fp_filter_count; e++) {
				if (string_endswith(entry->name, fp_filters[e].ext, false)) {
					valid = true;
					break;
				}
			}
		} else valid = true;

		if (valid) {
			fp_item_t item;
			item.name    = string_copy(entry->name);
			item.is_file = is_file;
			item.size    = (int64_t)entry->size;
			fp_items.add(item);
		}
		return true; // continue iteration
		});
	fp_items.sort([](const fp_item_t& a, const fp_item_t& b) { return (fp_sort_order_asc ? 1 : -1) * ((a.is_file != b.is_file) ? a.is_file - b.is_file : strcmp(a.name, b.name)); });

	char *new_folder = string_copy(folder);
	sk_free(fp_path.folder);
	for (int32_t i = 0; i < fp_path.fragments.count; i++) sk_free(fp_path.fragments[i]);
	fp_path.fragments.clear();

	fp_path.folder   = new_folder;
	fp_active        = nullptr;
	fp_scroll_offset = 0;
	fp_scroll_val    = 0;

	// Create path fragments for navigation
	char *curr       = fp_path.folder;
	char *curr_start = curr;
	while (*curr != '\0') {
		if (*curr == '/' || *curr == '\\') {
			if (curr_start != curr)
				fp_path.fragments.add( string_substr(curr_start, 0, (uint32_t)(curr - curr_start)) );
			curr_start = curr + 1;
		}
		curr++;
	}
	if (curr_start != curr) {
		fp_path.fragments.add(string_substr(curr_start, 0, (uint32_t)(curr - curr_start)));
	}
}

///////////////////////////////////////////

void file_picker_finish() {
	if (fp_callback) fp_callback(fp_call_data, fp_call_status, fp_filename, (int32_t)(strlen(fp_filename)+1));
	fp_call_status = false;
	fp_callback    = nullptr;
	fp_call_data   = nullptr;
	fp_call        = false;
	fp_show        = false;
	sprite_release(spr_folder); spr_folder = nullptr;
	sprite_release(spr_up    ); spr_up     = nullptr;
	sprite_release(spr_down  ); spr_down   = nullptr;
	sprite_release(spr_list  ); spr_list   = nullptr;
	sprite_release(spr_grid  ); spr_grid   = nullptr;
}

///////////////////////////////////////////

void file_picker_click_item(fp_item_t item) {
	if (item.is_file)
		fp_active = item.name;
	else {
		char* path = platform_push_path_new(fp_path.folder, item.name);
		file_picker_open_folder(path);
		sk_free(path);
	}
}

///////////////////////////////////////////

void file_picker_update() {
	if (fp_show) {
		float line_height = ui_line_height();
		vec2  size        = { .12f, line_height * 1.5f };

		ui_push_id("_skp");
		hierarchy_push(render_get_cam_root());
		ui_window_begin(fp_title, &fp_win_pose, { size.x * 3 + ui_get_gutter() * 3 + line_height + ui_get_margin()*2,0 }, ui_win_normal, ui_system_get_move_type());

		// Show the current directory address bar!
		vec3    address_bar_start = ui_layout_at();
		float   max_width         = ui_layout_remaining().x;
		float   width             = 0;
		int32_t start             = maxi(0, fp_path.fragments.count - 1);

		const float gutter  = ui_get_gutter();
		const float padding = ui_get_padding();
		bool should_change_list_mode = false;

		// Start at the end, and look backwards until we rin out of room!
		// That's the fragment we'll start with
		if (fp_path.fragments.count > 0) {
			for (int32_t i = start; i >= 0; i--) {
				float curr_size = fminf(max_width / 4, text_size_layout(fp_path.fragments[i]).x + padding * 2);
				if (width + curr_size > max_width) {
					break;
				}
				start = i;
				width += curr_size + gutter;
			}
		}
		// Draw the fragment crumbs as clickable buttons
		if (fp_path.fragments.count == 0) ui_layout_reserve(vec2{ max_width, line_height });
		for (int32_t i = start; i < fp_path.fragments.count; i++) {
			ui_push_idi(i);
			vec2 curr_size = { fminf(max_width / 4, text_size_layout(fp_path.fragments[i]).x + padding * 2), line_height };
			if (ui_button_sz(fp_path.fragments[i], curr_size) && i < fp_path.fragments.count - 1) {
				char* new_path = string_copy(fp_path.folder);
				for (int32_t p = i; p < fp_path.fragments.count - 1; p++)
				{
					char* next_path = platform_pop_path_new(new_path);
					sk_free(new_path);
					new_path = next_path;
				}
				file_picker_open_folder(new_path);
				sk_free(new_path);
				ui_pop_id();
				ui_sameline();
				break;
			}
			ui_pop_id();
			ui_sameline();
		}
		ui_panel_at(address_bar_start, vec2{ max_width, ui_line_height() });
		ui_nextline();

		// Show the active item
		switch (fp_mode) {
		case picker_mode_save: {
			if (ui_button("Cancel")) {
				fp_call        = true;
				fp_call_status = false;
			}
			ui_sameline();
			if (ui_button("Save")) {
				snprintf(fp_filename, sizeof(fp_filename), "%s%c%s", fp_path.folder, platform_path_separator_c, fp_buffer);
				fp_call        = true;
				fp_call_status = true;
			}
			ui_sameline();
			ui_input("SaveFile", fp_buffer, sizeof(fp_buffer), vec2{ ui_layout_remaining().x, ui_line_height() });
		} break;
		case picker_mode_open: {
			if (ui_button("Cancel")) { fp_call = true; fp_call_status = false; }
			ui_sameline();
			ui_push_enabled(fp_active != nullptr);
			if (ui_button("Open")) { snprintf(fp_filename, sizeof(fp_filename), "%s%c%s", fp_path.folder, platform_path_separator_c, fp_active); fp_call = true; fp_call_status = true; }
			ui_sameline();
			ui_text(fp_active ? fp_active : "None selected...", nullptr, ui_scroll_none, ui_line_height(), align_center_left, text_fit_none);
			ui_pop_enabled();
		} break;
		}

		ui_hseparator();

		const int32_t scroll_cols = 3;
		const int32_t scroll_rows = 5;
		const int32_t scroll_step = fp_list_mode ? scroll_rows : scroll_cols * scroll_rows;
		float right  = (size.x + gutter) * scroll_cols;

		// Show a header with sorting options
		if (fp_list_mode) {
			ui_btn_layout_ layout   = ui_btn_layout_left;
			vec2           btn_size = { 2 * size.x + ui_get_gutter(), 0.0f };
			if (fp_sortby == fp_sort_by_name) {
				if (ui_button_img_sz("Name", fp_sort_order_asc ? spr_up : spr_down, layout, btn_size)) {
					fp_sortby             = fp_sort_by_name;
					fp_sort_order_changed = true;
					fp_sort_order_asc     = !fp_sort_order_asc;
				}
			} else {
				if (ui_button_sz("Name", btn_size)) {
					fp_sortby             = fp_sort_by_name;
					fp_sort_order_changed = true;
					fp_sort_order_asc     = true;
				}
			}
			ui_sameline();
			if (fp_sortby == fp_sort_by_size) {
				if (ui_button_img_sz("Size", fp_sort_order_asc ? spr_up : spr_down, layout, { size.x, 0.0f })) {
					fp_sortby             = fp_sort_by_size;
					fp_sort_order_changed = true;
					fp_sort_order_asc     = !fp_sort_order_asc;
				}
			} else {
				if (ui_button_sz("Size", { size.x, 0.0f })) {
					fp_sortby             = fp_sort_by_size;
					fp_sort_order_changed = true;
					fp_sort_order_asc     = true;
				}
			}
			ui_sameline();
		}
		ui_hspace(ui_layout_remaining().x - (max_width - right));
		if (ui_button_img_sz("List", fp_list_mode ? spr_list : spr_grid, ui_btn_layout_center_no_text, { max_width - right, 0})) {
			should_change_list_mode = true;
			fp_scroll_offset = 0;
		}

		// Show the scrollable list of files
		vec3 file_grid_start = ui_layout_at();
		ui_panel_begin();
		for (int32_t i = fp_scroll_offset; i < fp_scroll_offset + scroll_step; i++) {
			if (i >= fp_items.count) {
				ui_layout_reserve({ fp_list_mode ? size.x * 3 + 2*ui_get_gutter() : size.x, size.y }, false, 0.0f);
				ui_sameline();
				continue;
			}

			sprite_t       sprite = nullptr;
			ui_btn_layout_ layout = ui_btn_layout_none;
			if (fp_items[i].is_file == false) {
				sprite = spr_folder;
				layout = ui_btn_layout_left;
			}

			if (fp_list_mode) {
				if (ui_button_img_sz(fp_items[i].name, sprite, layout, { 2*size.x+ui_get_gutter(), size.y })) {
					file_picker_click_item(fp_items[i]);
				}
				ui_sameline();
				if (fp_items[i].is_file) {
					char buffer[128];
					snprintf(buffer, sizeof(buffer), "%d KB ", (int32_t)(fp_items[i].size / 1024));
					ui_text_sz(buffer, nullptr, ui_scroll_none, size, align_center_right, text_fit_clip);
					ui_sameline();
				} else {
					ui_layout_reserve(size);
				}
			} else {
				if (ui_button_img_sz(fp_items[i].name, sprite, layout, size)) {
					file_picker_click_item(fp_items[i]);
				}
			}
			ui_sameline();
		}
		ui_panel_end();

		ui_nextline();

		// Show the scroll bar
		ui_push_enabled(fp_list_mode
			? fp_items.count >= scroll_rows
			: fp_items.count >= scroll_rows * scroll_cols);
		if (ui_vslider_at("scroll", fp_scroll_val, 0, (float)maxi(0,fp_items.count-3), 0, file_grid_start - vec3{ right,0,0 }, vec2{ max_width - right, size.y * scroll_rows + ui_get_gutter() * (scroll_rows-1) })) {
			fp_scroll_offset = fp_list_mode
				? (int32_t)fp_scroll_val
				: ((int32_t)fp_scroll_val / 3) * 3;
		}
		ui_pop_enabled();

		ui_window_end();
		hierarchy_pop();
		ui_pop_id();

		// Some deferred actions, these are deferred so they won't mess with
		// the UI flow when they change in the middle of the UI.
		if (should_change_list_mode) {
			fp_list_mode = !fp_list_mode;
		}
		if (fp_sort_order_changed) {
			switch (fp_sortby) {
			case fp_sort_by_name:
				fp_items.sort([](const fp_item_t& a, const fp_item_t& b) { return (fp_sort_order_asc ? 1 : -1) * ((a.is_file != b.is_file) ? a.is_file - b.is_file : strcmp(a.name, b.name)); });
				break;
			case fp_sort_by_size:
				fp_items.sort([](const fp_item_t& a, const fp_item_t& b) { return (fp_sort_order_asc ? 1 : -1) * (a.size == b.size ? strcmp(a.name, b.name) : a.size - b.size > 0 ? 1 : -1);});
				break;
			}
		}
	}

	if (fp_call) {
		file_picker_finish();
	}
}

///////////////////////////////////////////

void file_picker_shutdown() {
	sk_free(fp_title      );
	sk_free(fp_path.folder);
	for (int32_t i = 0; i < fp_path.fragments.count; i++) sk_free(fp_path.fragments[i]);
	fp_path.fragments.free();
	fp_path = {};
	fp_items.free();

	// Clear any pending native dialog state
	fp_native_dialog_id     = 0;
	fp_native_callback_data = nullptr;
	fp_native_callback      = nullptr;
}

///////////////////////////////////////////

void file_picker_handle_dialog_event(ska_event_file_dialog_t* evt) {
	// Check if this event matches our pending dialog
	if (evt->id != fp_native_dialog_id || fp_native_callback == nullptr) {
		ska_file_dialog_free_result(evt);
		return;
	}

	// Invoke the callback with the result
	if (evt->cancelled || evt->count == 0) {
		fp_native_callback(fp_native_callback_data, false, nullptr, 0);
	} else {
		const char* path = ska_file_dialog_get_path(evt, 0);
		if (path) {
			fp_native_callback(fp_native_callback_data, true, path, (int32_t)(strlen(path) + 1));
		} else {
			fp_native_callback(fp_native_callback_data, false, nullptr, 0);
		}
	}

	// Clean up
	ska_file_dialog_free_result(evt);
	fp_native_dialog_id     = 0;
	fp_native_callback_data = nullptr;
	fp_native_callback      = nullptr;
}

///////////////////////////////////////////

}