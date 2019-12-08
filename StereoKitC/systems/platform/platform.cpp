#include "platform.h"

#include "../../_stereokit.h"
#include "win32.h"
#include "uwp.h"
#include "openxr.h"

namespace sk {

///////////////////////////////////////////

bool platform_init() {
#if SK_NO_FLATSCREEN
	if (sk_runtime == runtime_flatscreen) {
		log_err("Flatscreen support has been compiled out of this configuration. Try a non-UWP x64 build, or switch to mixed reality!");
		throw "Flatscreen support has been compiled out of this configuration. Try a non-UWP x64 build, or switch to mixed reality!";
	}
#endif
	
	// Create a runtime
	bool result = sk_runtime == runtime_mixedreality ?
		openxr_init(sk_app_name) :
#ifndef SK_NO_FLATSCREEN
#if WINDOWS_UWP
		uwp_init(sk_app_name);
#else
		win32_init(sk_app_name);
#endif
#else
		false;
#endif

	if (!result)
		log_warnf("Couldn't create StereoKit in %s mode!", sk_runtime == runtime_mixedreality ? "MixedReality" : "Flatscreen");

	// Try falling back to flatscreen, if we didn't just try it
#ifndef SK_NO_FLATSCREEN
	if (!result && sk_runtime_fallback && sk_runtime != runtime_flatscreen) {
		log_infof("Runtime falling back to Flatscreen");
		sk_runtime = runtime_flatscreen;
#if WINDOWS_UWP
		result     = uwp_init   (sk_app_name);
#else
		result     = win32_init (sk_app_name);
#endif
	}
#endif
	return result;
}

///////////////////////////////////////////

void platform_shutdown() {
	switch (sk_runtime) {
#ifndef SK_NO_FLATSCREEN
#if WINDOWS_UWP
	case runtime_flatscreen:   uwp_shutdown   (); break;
#else
	case runtime_flatscreen:   win32_shutdown (); break;
#endif
#endif
	case runtime_mixedreality: openxr_shutdown(); break;
	}
}

///////////////////////////////////////////

void platform_begin_frame() {
	switch (sk_runtime) {
#ifndef SK_NO_FLATSCREEN
#if WINDOWS_UWP
	case runtime_flatscreen:   uwp_step_begin   (); break;
#else
	case runtime_flatscreen:   win32_step_begin (); break;
#endif
#endif
	case runtime_mixedreality: openxr_step_begin(); break;
	}
}

///////////////////////////////////////////

void platform_end_frame() {
	switch (sk_runtime) {
#ifndef SK_NO_FLATSCREEN
#if WINDOWS_UWP
	case runtime_flatscreen:   uwp_step_end   (); break;
#else
	case runtime_flatscreen:   win32_step_end (); break;
#endif
#endif
	case runtime_mixedreality: openxr_step_end(); break;
	}
}

///////////////////////////////////////////

void platform_present() {
	switch (sk_runtime) {
#ifndef SK_NO_FLATSCREEN
#if WINDOWS_UWP
	case runtime_flatscreen:   uwp_vsync  (); break;
#else
	case runtime_flatscreen:   win32_vsync(); break;
#endif
#endif
	case runtime_mixedreality: break;
	}
}

} // namespace sk