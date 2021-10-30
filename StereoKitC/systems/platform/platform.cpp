#include "platform.h"
#include "platform_utils.h"

#include "../../_stereokit.h"
#include "../../log.h"
#include "win32.h"
#include "uwp.h"
#include "linux.h"
#include "android.h"
#include "openxr.h"

namespace sk {

display_mode_ platform_mode = display_mode_none;

///////////////////////////////////////////

bool platform_init() {
	// Set up any platform dependant variables
#if   defined(SK_OS_ANDROID)
	bool result = android_init();
#elif defined(SK_OS_LINUX)
	bool result = linux_init  ();
#elif defined(SK_OS_WINDOWS_UWP)
	bool result = uwp_init    ();
#elif defined(SK_OS_WINDOWS)
	bool result = win32_init  ();
#endif
	if (!result) {
		log_fail_reason(80, log_error, "Platform initialization failed!");
		return false;
	}

	// Initialize graphics
	void *luid = sk_display_mode == display_mode_mixedreality 
		? openxr_get_luid() 
		: nullptr;
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
	if (!platform_set_mode(sk_display_mode)) {
		if (!sk_no_flatscreen_fallback && sk_display_mode != display_mode_flatscreen) {
			log_infof("MixedReality display mode failed, falling back to Flatscreen");
			sk_display_mode = display_mode_flatscreen;
			return platform_set_mode(sk_display_mode);
		}
		log_errf("Couldn't initialize StereoKit in %s mode!", sk_display_mode == display_mode_mixedreality ? "MixedReality" : "Flatscreen");
		return false;
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
	if (platform_mode == mode)
		return true;

	switch (mode) {
	case display_mode_none:         log_diag("Starting headless mode"); break;
	case display_mode_mixedreality: log_diag("Starting mixed reality mode"); break;
	case display_mode_flatscreen:   log_diag("Starting flatscreen mode"); break;
	}

	platform_stop_mode();

	bool result = true;
	if (mode == display_mode_mixedreality) {

		// Platform init before OpenXR
#if defined(SK_OS_ANDROID)
			result = android_start_pre_xr();
#elif defined(SK_OS_LINUX)
			result = linux_start_pre_xr();
#elif defined(SK_OS_WINDOWS_UWP)
			result = uwp_start_pre_xr();
#elif defined(SK_OS_WINDOWS)
			result = win32_start_pre_xr();
#endif

		// Init OpenXR
		if (result) {
			result = openxr_init ();
		}

		// Platform init after OpenXR
		if (result) {
#if defined(SK_OS_ANDROID)
			result = android_start_post_xr();
#elif defined(SK_OS_LINUX)
			result = linux_start_post_xr();
#elif defined(SK_OS_WINDOWS_UWP)
			result = uwp_start_post_xr();
#elif defined(SK_OS_WINDOWS)
			result = win32_start_post_xr();
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
#endif
	}

	platform_mode = mode;
	return result;
}
///////////////////////////////////////////


void platform_step_begin() {
	switch (platform_mode) {
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
#endif
		openxr_step_begin();
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
#endif
	} break;
	}
	platform_utils_update();
}

///////////////////////////////////////////

void platform_step_end() {
	switch (platform_mode) {
	case display_mode_none: break;
	case display_mode_mixedreality: openxr_step_end(); break;
	case display_mode_flatscreen: {
#if   defined(SK_OS_ANDROID)
		android_step_end_flat();
#elif defined(SK_OS_LINUX)
		linux_step_end_flat    ();
#elif defined(SK_OS_WINDOWS_UWP)
		uwp_step_end_flat    ();
#elif defined(SK_OS_WINDOWS)
		win32_step_end_flat  ();
#endif
	} break;
	}
}

///////////////////////////////////////////

void platform_stop_mode() {
	switch (platform_mode) {
	case display_mode_none: break;
	case display_mode_mixedreality: openxr_shutdown(); break;
	case display_mode_flatscreen: {
#if   defined(SK_OS_ANDROID)
		android_stop_flat();
#elif defined(SK_OS_LINUX)
		linux_stop_flat  ();
#elif defined(SK_OS_WINDOWS_UWP)
		uwp_stop_flat    ();
#elif defined(SK_OS_WINDOWS)
		win32_stop_flat  ();
#endif
	} break;
	}
}

} // namespace sk
