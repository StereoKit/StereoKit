// SPDX-License-Identifier: MIT
// The authors below grant copyright rights under the MIT license:
// Copyright (c) 2019-2023 Nick Klingensmith
// Copyright (c) 2023 Qualcomm Technologies, Inc.

#include "platform.h"
#include "platform_utils.h"
#include "../device.h"

#include "../_stereokit.h"
#include "../sk_memory.h"
#include "../log.h"
#include "../libraries/stref.h"
#include "../libraries/tinycthread.h"
#include "../systems/input.h"
#include "../xr_backends/openxr.h"
#include "../xr_backends/simulator.h"
#include "../xr_backends/window.h"
#include "../xr_backends/offscreen.h"
#include "../xr_backends/xr.h"

#include "../libraries/sk_gpu.h"

///////////////////////////////////////////

using namespace sk;

struct platform_state_t {
	app_mode_ mode;
	thrd_id_t gpu_thread;
};
static platform_state_t* local = {};

///////////////////////////////////////////

namespace sk {

///////////////////////////////////////////

const char* app_mode_str(app_mode_ mode);

///////////////////////////////////////////

bool platform_init() {
	device_data_init(&device_data);

	local = sk_malloc_zero_t(platform_state_t, 1);

	// Set up any platform dependent variables
	if (!platform_impl_init()) {
		log_fail_reason(80, log_error, "Platform initialization failed!");
		return false;
	}

	const sk_settings_t *settings = sk_get_settings_ref();

	// Initialize graphics
#if defined(SK_XR_OPENXR)
	void *luid = settings->mode == app_mode_xr
		? openxr_get_luid()
		: nullptr;
#else
	void *luid = nullptr;
#endif
	skg_callback_log([](skg_log_ level, const char *text) {
		switch (level) {
		case skg_log_info:     log_diagf("[<~mag>sk_gpu<~clr>] %s", text); break;
		case skg_log_warning:  log_warnf("[<~mag>sk_gpu<~clr>] %s", text); break;
		case skg_log_critical: log_errf ("[<~mag>sk_gpu<~clr>] %s", text); break;
		}
	});
	if (skg_init(settings->app_name, luid) <= 0) {
		log_fail_reason(95, log_error, "Failed to initialize sk_gpu!");
		return false;
	}
	local->gpu_thread = thrd_id_current();
	device_data.gpu   = string_copy(skg_adapter_name());

	// Start up the current mode!
	if (!platform_set_mode(settings->mode)) {
		if (!settings->no_flatscreen_fallback && settings->mode != app_mode_simulator) {
			log_clear_any_fail_reason();
			log_infof("Couldn't create an XR app, falling back to Simulator");
			if (!platform_set_mode(app_mode_simulator))
				return false;
		} else {
			log_errf("Couldn't initialize a <~grn>%s<~clr> mode app!", app_mode_str(settings->mode));
			return false;
		}
	}
	return platform_utils_init();
}

///////////////////////////////////////////

void platform_shutdown() {
	platform_utils_shutdown();
	platform_stop_mode();
	skg_shutdown();

	platform_impl_shutdown();

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

	log_diagf("Starting a <~grn>%s<~clr> mode app", app_mode_str(local->mode));
	switch (local->mode) {
	case app_mode_offscreen: return offscreen_init(); break;
	case app_mode_simulator: return simulator_init(); break;
	case app_mode_window:    return window_init   (); break;
	case app_mode_xr:        return xr_init       (); break;
	}

	return false;
}
///////////////////////////////////////////


void platform_step_begin() {
	platform_impl_step();
	switch (local->mode) {
	case app_mode_offscreen: offscreen_step_begin(); break;
	case app_mode_simulator: simulator_step_begin(); break;
	case app_mode_window:    window_step_begin   (); break;
	case app_mode_xr:        xr_step_begin       (); break;
	}
	platform_utils_update();
}

///////////////////////////////////////////

void platform_step_end() {
	switch (local->mode) {
	case app_mode_offscreen: offscreen_step_end(); break;
	case app_mode_simulator: simulator_step_end(); break;
	case app_mode_window:    window_step_end   (); break;
	case app_mode_xr:        xr_step_end       (); break;
	}
}

///////////////////////////////////////////

void platform_stop_mode() {
	switch (local->mode) {
	case app_mode_offscreen: offscreen_shutdown(); break;
	case app_mode_simulator: simulator_shutdown(); break;
	case app_mode_window:    window_shutdown   (); break;
	case app_mode_xr:        xr_shutdown       (); break;
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

bool platform_is_gpu_thread() {
	return thrd_id_equal(local->gpu_thread, thrd_id_current());
}

} // namespace sk
