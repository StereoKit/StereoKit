/* SPDX-License-Identifier: MIT */
/* The authors below grant copyright rights under the MIT license:
 * Copyright (c) 2019-2023 Nick Klingensmith
 * Copyright (c) 2023 Qualcomm Technologies, Inc.
 */

#include "_platform.h"

#include <sk_app.h>
#include <sk_renderer.h>

#include "../device.h"
#include "../_stereokit.h"
#include "../sk_memory.h"
#include "../sk_math.h"
#include "../log.h"
#include "../libraries/stref.h"
#include "../libraries/ferr_thread.h"
#include "../libraries/profiler.h"
#include "../libraries/array.h"
#include "../xr_backends/openxr.h"
#include "../xr_backends/extensions/vulkan_enable.h"
#include "../xr_backends/simulator.h"
#include "../xr_backends/window.h"
#include "../xr_backends/offscreen.h"
#include "../xr_backends/xr.h"
#include "../platforms/android.h"

#include "../systems/input_keyboard.h"
#include "../tools/virtual_keyboard.h"
#include "../tools/file_picker.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#if defined(SK_OS_WINDOWS)

	#ifndef WIN32_LEAN_AND_MEAN
	#define WIN32_LEAN_AND_MEAN
	#endif
	#include <windows.h>

#endif
#include "../asset_types/assets.h"

///////////////////////////////////////////

using namespace sk;

struct platform_state_t {
	app_mode_ mode;
	bool32_t  force_fallback_keyboard;
};
static platform_state_t* local = {};

///////////////////////////////////////////
// Allocator wrappers for sk_app
///////////////////////////////////////////

static void* ska_alloc_wrapper(size_t size, void* user_data) {
	(void)user_data;
	return sk_malloc(size);
}
static void* ska_realloc_wrapper(void* ptr, size_t size, void* user_data) {
	(void)user_data;
	return sk_realloc(ptr, size);
}
static void ska_free_wrapper(void* ptr, void* user_data) {
	(void)user_data;
	_sk_free(ptr);
}

///////////////////////////////////////////

namespace sk {

///////////////////////////////////////////

const char* app_mode_str      (app_mode_ mode);
bool        platform_set_mode (app_mode_ mode);
void        platform_stop_mode();

///////////////////////////////////////////

bool platform_init() {
	profiler_zone();

	device_data_init(&device_data);

	local = sk_malloc_zero_t(platform_state_t, 1);
	const sk_settings_t* settings = sk_get_settings_ref();

	// Initialize sk_app for platform abstraction (file I/O, windowing, etc.)
	// sk_app handles cross-platform window management and input for non-XR modes
	ska_settings_t ska_settings = {};
	ska_settings.alloc   = ska_alloc_wrapper;
	ska_settings.realloc = ska_realloc_wrapper;
	ska_settings.free    = ska_free_wrapper;
	if (!ska_init(&ska_settings)) {
		log_errf("sk_app initialization failed: %s", ska_error_get() ? ska_error_get() : "unknown error");
		log_fail_reason(80, log_error, "sk_app initialization failed");
		return false;
	}
	ska_kvpstore_set_app_name(settings->app_name ? settings->app_name : "StereoKit");

	// Set up any platform dependent variables
	if (!platform_impl_init()) {
		log_fail_reason(80, log_error, "Platform initialization failed!");
		ska_shutdown();
		return false;
	}

	// Initialize graphics with sk_renderer
	skr_callback_log([](skr_log_ level, const char *text) {
		switch (level) {
		case skr_log_info:     log_diagf("[<~ylw>sk_renderer<~clr>] %s", text); break;
		case skr_log_warning:  log_warnf("[<~ylw>sk_renderer<~clr>] %s", text); break;
		case skr_log_critical: log_errf ("[<~ylw>sk_renderer<~clr>] %s", text); break;
		}
	});


	skr_bind_settings_t skr_binds = {};
	skr_binds.instance_slot = 12; // Instance data binds to slot t12
	skr_binds.material_slot = 0;  // Material data binds to $Globals, which is slot b0
	skr_binds.system_slot   = 1;  // StereoKit's system data goes in slot b1
	skr_settings_t skr_settings = {};
	skr_settings.app_name          = settings->app_name;
	skr_settings.app_version       = 1;
	skr_settings.enable_validation = true;// settings->log_filter == log_diagnostic;
	skr_settings.bind_settings     = &skr_binds;

	// Build extension array - start with platform-specific surface extensions from sk_app
	array_t<const char*> vk_extensions = {};
	uint32_t     ska_ext_count = 0;
	const char** ska_exts      = ska_vk_get_instance_extensions(&ska_ext_count);
	if (ska_exts != nullptr) {
		vk_extensions.add_range(ska_exts, ska_ext_count);
	}

	// For XR modes, initialize OpenXR early to get Vulkan requirements
#if defined(SK_XR_OPENXR)
	if (settings->mode == app_mode_xr) {
		const char** xr_exts      = nullptr;
		uint32_t     xr_ext_count = 0;
		if (openxr_create_system() && xr_ext_vulkan_enable_setup_skr(&skr_settings, &xr_exts, &xr_ext_count)) {
			vk_extensions.add_range(xr_exts, xr_ext_count);
		}
	}
#endif

	skr_settings.required_extensions      = vk_extensions.data;
	skr_settings.required_extension_count = (uint32_t)vk_extensions.count;

	bool skr_result = skr_init(skr_settings);
	vk_extensions.free();
	if (!skr_result) {
		log_fail_reason(95, log_error, "Failed to initialize sk_renderer!");
		return false;
	}

	// Get GPU name from Vulkan physical device properties
	VkPhysicalDeviceProperties props;
	vkGetPhysicalDeviceProperties(skr_get_vk_physical_device(), &props);
	device_data.gpu = string_copy(props.deviceName);

	// Start up the current mode!
	bool result = platform_set_mode(settings->mode);
	if (result == false && settings->no_flatscreen_fallback == false && settings->mode != app_mode_simulator) {
		log_clear_any_fail_reason();
		log_infof("Couldn't create an XR app, falling back to Simulator");
		result = platform_set_mode(app_mode_simulator);
	}
	if (result == false)
		log_errf("Couldn't initialize a <~grn>%s<~clr> mode app!", app_mode_str(settings->mode));
	return result;
}

///////////////////////////////////////////

void platform_shutdown() {
	platform_stop_mode();

	platform_impl_shutdown();

	// Shutdown sk_app after platform impl to ensure windows are destroyed first
	ska_shutdown();

	device_data_free(&device_data);
	*local = {};
	sk_free(local);
}

///////////////////////////////////////////

bool platform_set_mode(app_mode_ mode) {
	if (local->mode == mode)          return true;
	if (mode        == app_mode_none) return false;

	platform_stop_mode();
	local->mode = mode;
	sk_get_settings_ref_mut()->mode = mode;

	log_diagf("Starting a <~grn>%s<~clr> mode app", app_mode_str(local->mode));
	switch (local->mode) {
	case app_mode_offscreen: return offscreen_init(); break;
	case app_mode_simulator: return simulator_init(); break;
	case app_mode_window:    return window_init   (); break;
	case app_mode_xr:        return xr_init       (); break;
	case app_mode_none:      return false;
	}

	return false;
}
///////////////////////////////////////////


void platform_step_begin() {
	profiler_zone();

	platform_impl_step();
	switch (local->mode) {
	case app_mode_offscreen: offscreen_step_begin(); break;
	case app_mode_simulator: simulator_step_begin(); break;
	case app_mode_window:    window_step_begin   (); break;
	case app_mode_xr:        xr_step_begin       (); break;
	case app_mode_none:      break;
	}
}

///////////////////////////////////////////

void platform_step_end() {
	profiler_zone();

	switch (local->mode) {
	case app_mode_offscreen: offscreen_step_end(); break;
	case app_mode_simulator: simulator_step_end(); break;
	case app_mode_window:    window_step_end   (); break;
	case app_mode_xr:        xr_step_end       (); break;
	case app_mode_none:      break;
	}
}

///////////////////////////////////////////

void platform_stop_mode() {
	switch (local->mode) {
	case app_mode_offscreen: offscreen_shutdown(); break;
	case app_mode_simulator: simulator_shutdown(); break;
	case app_mode_window:    window_shutdown   (); break;
	case app_mode_xr:        xr_shutdown       (); break;
	case app_mode_none:      break;
	}
}

///////////////////////////////////////////

const char* app_mode_str(app_mode_ mode) {
	switch (mode) {
	case app_mode_none:      return "None";
	case app_mode_simulator: return "Simulator";
	case app_mode_window:    return "Window";
	case app_mode_offscreen: return "Offscreen";
	case app_mode_xr:        return "XR";
	}
	return "";
}

///////////////////////////////////////////

void platform_set_window(void *window) {
#if defined(SK_OS_ANDROID)
	android_set_window(window);
#else
	(void)window;
#endif
}

///////////////////////////////////////////

void platform_set_window_xam(void *window) {
#if defined(SK_OS_ANDROID)
	android_set_window_xam(window);
#else
	(void)window;
#endif
}

///////////////////////////////////////////


bool32_t platform_keyboard_get_force_fallback() {
	return local->force_fallback_keyboard;
}

///////////////////////////////////////////

void platform_keyboard_set_force_fallback(bool32_t force_fallback) {
	local->force_fallback_keyboard = force_fallback;
}

///////////////////////////////////////////

void platform_keyboard_show(bool32_t visible, text_context_ type) {
	// If we're _forcing_ the fallback keyboard, this becomes quite simple!
	if (local->force_fallback_keyboard) {
		virtualkeyboard_open(visible, type);
		return;
	}

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
	else if (device_display_get_type() != display_type_flatscreen &&
	         (input_get_last_physical_keypress_time() < 0 || (time_totalf_unscaled()-input_get_last_physical_keypress_time()) > physical_interact_timeout) ) {

		virtualkeyboard_open(visible, type);
	}
}

bool32_t platform_keyboard_set_layout(text_context_ keyboard_type, const char** keyboard_text, int layouts_num) {
	return virtualkeyboard_set_layout(keyboard_type, keyboard_text, layouts_num);
}

///////////////////////////////////////////

bool32_t platform_keyboard_visible() {
	return virtualkeyboard_get_open();
}

///////////////////////////////////////////

bool platform_file_exists(const char *filename) {
	struct stat buffer;
	return (stat (filename, &buffer) == 0);
}

///////////////////////////////////////////

bool platform_file_delete(const char* filename) {
	return remove(filename) == 0;
}

///////////////////////////////////////////

char *platform_push_path_ref(char *path, const char *directory) {
	char *result = platform_push_path_new(path, directory);
	sk_free(path);
	return result;
}

///////////////////////////////////////////

char *platform_pop_path_ref(char *path) {
	char *result = platform_pop_path_new(path);
	sk_free(path);
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
		result = string_append(nullptr, 2, path, &directory[1]);
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
#if defined(SK_OS_WINDOWS)
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

bool32_t platform_read_file_direct(const char *filename, void **out_data, size_t *out_size) {
	*out_data = nullptr;
	*out_size = 0;

	// Normalize slashes to platform-specific separator
	char* normalized = string_copy(filename);
	for (char* c = normalized; *c; c++) {
		if (*c == '\\' || *c == '/') *c = platform_path_separator_c;
	}

	bool success = false;

	// Try reading the file directly
	if (ska_file_exists(normalized)) {
		success = ska_file_read(normalized, out_data, out_size);
	}

	// On Android, try reading from APK assets
#if defined(SK_OS_ANDROID)
	if (!success) {
		success = ska_asset_read(normalized, out_data, out_size);
	}
#endif

	// If file not found and path is relative, try exe-relative
	if (!success && normalized[0] != platform_path_separator_c) {
#if defined(SK_OS_WINDOWS)
		bool is_absolute = (strlen(normalized) >= 2 && normalized[1] == ':');
		if (!is_absolute) {
#endif
		char exe_path[1024];
		if (ska_get_exe_path(exe_path, sizeof(exe_path))) {
			// ska_get_exe_path returns the exe file path, we need the directory
			char* exe_dir  = platform_pop_path_new(exe_path);
			char* fullpath = platform_push_path_new(exe_dir, normalized);
			if (ska_file_exists(fullpath)) {
				success = ska_file_read(fullpath, out_data, out_size);
			}
			sk_free(fullpath);
			sk_free(exe_dir);
		}
#if defined(SK_OS_WINDOWS)
		}
#endif
	}

	if (!success) {
		log_diagf("platform_read_file_direct can't find %s", normalized);
		sk_free(normalized);
		return false;
	}

	sk_free(normalized);

	// Add null terminator for string convenience
	*out_data = sk_realloc(*out_data, *out_size + 1);
	((uint8_t*)*out_data)[*out_size] = 0;

	return true;
}

///////////////////////////////////////////
bool32_t platform_read_file(const char* filename, void** out_data, size_t* out_size) {
	char*    asset_filename   = assets_file(filename);
	bool32_t read_file_result = platform_read_file_direct(asset_filename, out_data, out_size);
	sk_free(asset_filename);
	return read_file_result;
}

///////////////////////////////////////////

bool32_t platform_write_file(const char* filename, void* data, size_t size) {
	if (!ska_file_write(filename, data, size)) {
		log_diagf("platform_write_file can't write %s", filename);
		return false;
	}
	return true;
}
bool32_t platform_write_file_text(const char* filename, const char* text) {
	return platform_write_file(filename, (void*)text, strlen(text));
}

} // namespace sk
