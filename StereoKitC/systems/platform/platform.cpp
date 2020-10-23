#include "platform.h"

#include "../../_stereokit.h"
#include "win32.h"
#include "uwp.h"
#include "android.h"
#include "openxr.h"

namespace sk {

///////////////////////////////////////////

bool platform_init(void *from_window) {
#if __ANDROID__
	android_setup(from_window);
#elif WINDOWS_UWP
	uwp_setup    (from_window);
#elif _WIN32
	win32_setup  (from_window);
#endif

	// Create a runtime
	bool result = sk_runtime == runtime_mixedreality ?
		openxr_init (from_window) :
#if __ANDROID__
		android_init();
#if __linux__
		linux_init();
#elif WINDOWS_UWP
		uwp_init    ();
#elif _WIN32
		win32_init  ();
#endif

	if (!result)
		log_warnf("Couldn't create StereoKit in %s mode!", sk_runtime == runtime_mixedreality ? "MixedReality" : "Flatscreen");

	// Try falling back to flatscreen, if we didn't just try it
	if (!result && sk_runtime_fallback && sk_runtime != runtime_flatscreen) {
		log_infof("Runtime falling back to Flatscreen");
		sk_runtime = runtime_flatscreen;
#if __ANDROID__
		result     = android_init();
#elif WINDOWS_UWP
		result     = uwp_init    ();
#elif _WIN32
		result     = win32_init  ();
#endif
	}
	return result;
}

} // namespace sk
