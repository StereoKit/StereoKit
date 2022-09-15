#include "platform.h"
#include "platform_utils.h"

#include "../_stereokit.h"
#include "../log.h"
#include "win32.h"
#include "uwp.h"
#include "linux.h"
#include "android.h"
#include "web.h"
#include "../xr_backends/openxr.h"

#include "../libraries/sk_gpu.h"

namespace sk {

///////////////////////////////////////////

bool platform_init() {
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

	// Initialize graphics
#if defined(SK_XR_OPENXR)
	void *luid = sk_get_settings().display_preference == display_mode_mixedreality
		? openxr_get_luid() 
		: nullptr;
#else
	void *luid = nullptr;
#endif
	skg_callback_log([](skg_log_ level, const char *text) {
		switch (level) {
		case skg_log_info:     log_diagf("sk_gpu: %s", text); break;
		case skg_log_warning:  log_warnf("sk_gpu: %s", text); break;
		case skg_log_critical: log_errf ("sk_gpu: %s", text); break;
		}
	});
	if (skg_init(sk_app_name, luid) <= 0) {
		log_fail_reason(95, log_error, "Failed to initialize sk_gpu!");
		return false;
	}

	// Start up the current mode!
	if (!platform_set_mode(sk_get_settings().display_preference)) {
		if (!sk_no_flatscreen_fallback && sk_get_settings().display_preference != display_mode_flatscreen) {
			log_clear_any_fail_reason();
			log_infof("MixedReality display mode failed, falling back to Flatscreen");
			if (!platform_set_mode(display_mode_flatscreen))
				return false;
		} else {
			log_errf("Couldn't initialize StereoKit in %s mode!", sk_get_settings().display_preference == display_mode_mixedreality ? "MixedReality" : "Flatscreen");
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

bool platform_set_mode(display_mode_ mode) {
	if (sk_display_mode == mode)
		return true;

	switch (mode) {
	case display_mode_none:         log_diag("Starting headless mode"); break;
	case display_mode_mixedreality: log_diag("Starting mixed reality mode"); break;
	case display_mode_flatscreen:   log_diag("Starting flatscreen mode"); break;
	}

	platform_stop_mode();
	sk_display_mode = mode;

	bool result = true;
	if (mode == display_mode_mixedreality) {

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
	} else if (mode == display_mode_flatscreen) {
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
	}

	return result;
}
///////////////////////////////////////////


void platform_step_begin() {
	switch (sk_display_mode) {
	case display_mode_none: break;
	case display_mode_mixedreality: {
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
	} break;
	case display_mode_flatscreen: {
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
	} break;
	}
	platform_utils_update();
}

///////////////////////////////////////////

void platform_step_end() {
	switch (sk_display_mode) {
	case display_mode_none: break;
	case display_mode_mixedreality:
#if defined(SK_XR_OPENXR)
		openxr_step_end();
#else
#endif
		break;
	case display_mode_flatscreen: {
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
	} break;
	}
}

///////////////////////////////////////////

void platform_stop_mode() {
	switch (sk_display_mode) {
	case display_mode_none: break;
	case display_mode_mixedreality:
#if defined(SK_XR_OPENXR)
		openxr_shutdown();
#else
#endif
		break;
	case display_mode_flatscreen: {
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
	sk_display_mode = display_mode_none;
}

} // namespace sk
