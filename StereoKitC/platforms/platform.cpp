#include "platform.h"
#include "platform_utils.h"
#include "../device.h"

#include "../_stereokit.h"
#include "../log.h"
#include "../libraries/stref.h"
#include "../systems/input.h"
#include "win32.h"
#include "uwp.h"
#include "linux.h"
#include "android.h"
#include "web.h"
#include "../xr_backends/openxr.h"
#include "../xr_backends/simulator.h"
#include "../xr_backends/none.h"

#include "../libraries/sk_gpu.h"

namespace sk {

///////////////////////////////////////////

bool platform_init() {
	device_data_init(&device_data);

	// Set up any platform dependent variables
#if   defined(SK_OS_ANDROID)
	bool result = android_init();
#elif defined(SK_OS_LINUX)
	bool result = linux_init  ();
#elif defined(SK_OS_WINDOWS_UWP)
	bool result = uwp_init    ();
#elif defined(SK_OS_WINDOWS)
	bool result = win32_init  ();
#elif defined(SK_OS_WEB)
	bool result = web_init    ();
#endif
	if (!result) {
		log_fail_reason(80, log_error, "Platform initialization failed!");
		return false;
	}

	const sk_settings_t *settings = sk_get_settings_ref();

	// Initialize graphics
#if defined(SK_XR_OPENXR)
	void *luid = settings->display_preference == display_mode_mixedreality
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
	device_data.gpu = string_copy(skg_adapter_name());

	// Convert from the legacy display_mode_ type
	display_type_ display_type = display_type_none;
	switch (settings->display_preference) {
	case display_mode_flatscreen:   display_type = display_type_flatscreen; break;
	case display_mode_mixedreality: display_type = display_type_stereo;     break;
	case display_mode_none:         display_type = display_type_none;       break;
	}

	// Start up the current mode!
	if (!platform_set_mode(display_type)) {
		if (!settings->no_flatscreen_fallback && display_type != display_type_flatscreen) {
			log_clear_any_fail_reason();
			log_infof("Couldn't create a stereo display, falling back to flatscreen");
			if (!platform_set_mode(display_type_flatscreen))
				return false;
		} else {
			log_errf("Couldn't initialize a %s display!", display_type == display_type_stereo ? "stereo" : "flatscreen");
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

#if   defined(SK_OS_ANDROID)
	android_shutdown();
#elif defined(SK_OS_LINUX)
	linux_shutdown  ();
#elif defined(SK_OS_WINDOWS_UWP)
	uwp_shutdown    ();
#elif defined(SK_OS_WINDOWS)
	win32_shutdown  ();
#elif defined(SK_OS_WEB)
	web_shutdown    ();
#endif

	device_data_free(&device_data);
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

bool platform_set_mode(display_type_ mode) {
	if (device_data.display_type == mode)
		return true;

	switch (mode) {
	case display_type_none:       log_diagf("Starting a <~grn>%s<~clr> display", "headless"  ); break;
	case display_type_stereo:     log_diagf("Starting a <~grn>%s<~clr> display", "stereo"    ); break;
	case display_type_flatscreen: log_diagf("Starting a <~grn>%s<~clr> display", "flatscreen"); break;
	}

	platform_stop_mode();
	device_data.display_type = mode;

	bool result = true;
	if (mode == display_type_stereo) {

		// Platform init before OpenXR
#if defined(SK_OS_ANDROID)
			result = android_start_pre_xr();
#elif defined(SK_OS_LINUX)
			result = linux_start_pre_xr  ();
#elif defined(SK_OS_WINDOWS_UWP)
			result = uwp_start_pre_xr    ();
#elif defined(SK_OS_WINDOWS)
			result = win32_start_pre_xr  ();
#elif defined(SK_OS_WEB)
			result = web_start_pre_xr    ();
#endif

		// Init OpenXR
		if (result) {
#if defined(SK_XR_OPENXR)
			result = openxr_init ();
#else
			result = true;
#endif
		}

		// Platform init after OpenXR
		if (result) {
#if defined(SK_OS_ANDROID)
			result = android_start_post_xr();
#elif defined(SK_OS_LINUX)
			result = linux_start_post_xr  ();
#elif defined(SK_OS_WINDOWS_UWP)
			result = uwp_start_post_xr    ();
#elif defined(SK_OS_WINDOWS)
			result = win32_start_post_xr  ();
#elif defined(SK_OS_WEB)
			result = web_start_post_xr    ();
#endif
		}
	} else if (mode == display_type_flatscreen) {
#if   defined(SK_OS_ANDROID)
		result = android_start_flat();
#elif defined(SK_OS_LINUX)
		result = linux_start_flat  ();
#elif defined(SK_OS_WINDOWS_UWP)
		result = uwp_start_flat    ();
#elif defined(SK_OS_WINDOWS)
		result = win32_start_flat  ();
#elif defined(SK_OS_WEB)
		result = web_start_flat    ();
#endif
		if (sk_get_settings_ref()->disable_flatscreen_mr_sim) none_init();
		else                                                  simulator_init();
	}

	return result;
}
///////////////////////////////////////////


void platform_step_begin() {
	switch (device_data.display_type) {
	case display_type_none: break;
	case display_type_stereo: {
#if   defined(SK_OS_ANDROID)
		android_step_begin_xr();
#elif defined(SK_OS_LINUX)
		linux_step_begin_xr  ();
#elif defined(SK_OS_WINDOWS_UWP)
		uwp_step_begin_xr    ();
#elif defined(SK_OS_WINDOWS)
		win32_step_begin_xr  ();
#elif defined(SK_OS_WEB)
		web_step_begin_xr    ();
#endif

#if defined(SK_XR_OPENXR)
		openxr_step_begin();
#else
#endif
		input_step();
	} break;
	case display_type_flatscreen: {
#if   defined(SK_OS_ANDROID)
		android_step_begin_flat();
#elif defined(SK_OS_LINUX)
		linux_step_begin_flat  ();
#elif defined(SK_OS_WINDOWS_UWP)
		uwp_step_begin_flat    ();
#elif defined(SK_OS_WINDOWS)
		win32_step_begin_flat  ();
#elif defined(SK_OS_WEB)
		web_step_begin_flat    ();
#endif
		input_step();
		if (sk_get_settings_ref()->disable_flatscreen_mr_sim) none_step_begin();
		else                                                  simulator_step_begin();
	} break;
	}
	platform_utils_update();
}

///////////////////////////////////////////

void platform_step_end() {
	switch (device_data.display_type) {
	case display_type_none: break;
	case display_type_stereo:
#if defined(SK_XR_OPENXR)
		openxr_step_end();
#else
#endif
		break;
	case display_type_flatscreen: {
		if (sk_get_settings_ref()->disable_flatscreen_mr_sim) none_step_end();
		else                                                  simulator_step_end();
#if   defined(SK_OS_ANDROID)
		android_step_end_flat();
#elif defined(SK_OS_LINUX)
		linux_step_end_flat  ();
#elif defined(SK_OS_WINDOWS_UWP)
		uwp_step_end_flat    ();
#elif defined(SK_OS_WINDOWS)
		win32_step_end_flat  ();
#elif defined(SK_OS_WEB)
		web_step_end_flat    ();
#endif
		openxr_step_end();
	} break;
	}
}

///////////////////////////////////////////

void platform_stop_mode() {
	switch (device_data.display_type) {
	case display_type_none: break;
	case display_type_stereo:
#if defined(SK_XR_OPENXR)
		openxr_shutdown();
#else
#endif
		break;
	case display_type_flatscreen: {
		if (sk_get_settings_ref()->disable_flatscreen_mr_sim) none_shutdown();
		else                                                  simulator_shutdown();
#if   defined(SK_OS_ANDROID)
		android_stop_flat();
#elif defined(SK_OS_LINUX)
		linux_stop_flat  ();
#elif defined(SK_OS_WINDOWS_UWP)
		uwp_stop_flat    ();
#elif defined(SK_OS_WINDOWS)
		win32_stop_flat  ();
#elif defined(SK_OS_WEB)
		web_stop_flat    ();
#endif
	} break;
	}
	device_data.display_type = display_type_none;
}

} // namespace sk
