/* SPDX-License-Identifier: MIT */
/* The authors below grant copyright rights under the MIT license:
 * Copyright (c) 2019-2024 Nick Klingensmith
 * Copyright (c) 2023-2024 Qualcomm Technologies, Inc.
 */

#include "android.h"
#if defined(SK_OS_ANDROID)

#include "../log.h"
#include "../sk_memory.h"
#include "../_stereokit.h"
#include "../libraries/array.h"

// JNI types must be available before Vulkan Android / OpenXR Android headers
#include <jni.h>
#include <android/native_activity.h>
#include <android/native_window_jni.h>

#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>
#include <android/font_matcher.h>
#include <android/font.h>

#include <dlfcn.h>
#include <sk_app.h>

namespace sk {

// These are variables that are set outside of the normal initialization cycle,
// such as the vm from when the library loads, or window change events from the
// main activity.
struct platform_android_persistent_state_t {
	JavaVM        *vm;
	ANativeWindow *next_window;
	jobject        next_window_xam;
	bool           next_win_ready;
};

struct platform_android_state_t {
	JNIEnv        *env;
	jobject        activity;
	AAssetManager *asset_manager;
	jobject        asset_manager_obj;
	ANativeWindow *window;
};

static platform_android_state_t            local         = {};
static platform_android_persistent_state_t local_persist = {};

///////////////////////////////////////////

// When consumed as a static library, the user's app will need to provide its
// own JNI_OnLoad function, which will then conflict with this one. So we only
// provide this for shared libraries.
#if defined(SK_BUILD_SHARED)

extern "C" jint JNI_OnLoad(JavaVM* vm, void* reserved) {
	local_persist.vm = vm;
	return JNI_VERSION_1_6;
}
extern "C" jint JNI_OnLoad_L(JavaVM* vm, void* reserved) {
	return JNI_OnLoad(vm, reserved);
}

#endif

///////////////////////////////////////////

bool platform_impl_init() {
	const sk_settings_t* settings = sk_get_settings_ref();
	local = {};

	local.activity   = (jobject)settings->android_activity;
	if (local_persist.vm == nullptr)
		local_persist.vm = (JavaVM*)settings->android_java_vm;

	if (local_persist.vm == nullptr || local.activity == nullptr) {
		log_fail_reason(95, log_error, "Couldn't find Android's Java VM or Activity, you should load the StereoKitC library with something like Xamarin's JavaSystem.LoadLibrary, or manually assign it using sk_set_settings()");
		return false;
	}

	// Get the java environment from the VM, and ensure it's attached to this
	// thread
	int result = local_persist.vm->GetEnv((void **)&local.env, JNI_VERSION_1_6);
	if (result == JNI_EDETACHED) {
		if (local_persist.vm->AttachCurrentThread(&local.env, nullptr) != JNI_OK) {
			log_fail_reason(95, log_error, "Couldn't attach the Java Environment to the current thread!");
			return false;
		}
	} else if (result != JNI_OK) {
		log_fail_reason(95, log_error, "Couldn't get the Java Environment from the VM, this needs to be called from the main thread.");
		return false;
	}

	// Get the asset manager for loading files
	// from https://stackoverflow.com/questions/22436259/android-ndk-why-is-aassetmanager-open-returning-null/22436260#22436260
	jclass    activity_class           = local.env->GetObjectClass(local.activity);
	jmethodID activity_class_getAssets = local.env->GetMethodID(activity_class, "getAssets", "()Landroid/content/res/AssetManager;");
	jobject   asset_manager            = local.env->CallObjectMethod(local.activity, activity_class_getAssets); // activity.getAssets();
	local.asset_manager_obj            = local.env->NewGlobalRef(asset_manager);
	local.asset_manager = AAssetManager_fromJava(local.env, local.asset_manager_obj);
	if (local.asset_manager == nullptr) {
		log_fail_reason(95, log_error, "Couldn't get the Android asset manager!");
		return false;
	}
	local.env->DeleteLocalRef(activity_class);
	local.env->DeleteLocalRef(asset_manager);

#if defined(SK_DYNAMIC_OPENXR)
	// Android has no universally supported openxr_loader yet, so on this
	// platform we don't static link it, and instead provide devs a way to ship
	// other loaders.
	if (settings->mode == app_mode_xr && dlopen("libopenxr_loader.so", RTLD_NOW) == nullptr) {
		log_fail_reason(95, log_error, "openxr_loader failed to load!");
		return false;
	}
#endif

	return true;
}

///////////////////////////////////////////

void platform_impl_shutdown() {
	local.env->DeleteGlobalRef(local.asset_manager_obj);
}

///////////////////////////////////////////

void platform_impl_step() {
	if (!local_persist.next_win_ready) return;

	// If we got our window from xamarin, it's a jobject, and needs
	// converted into an ANativeWindow first!
	if (local_persist.next_window_xam != nullptr) {
		local_persist.next_window = ANativeWindow_fromSurface(local.env, local_persist.next_window_xam);
		local_persist.next_window_xam = nullptr;
	}

	local.window = local_persist.next_window;
	local_persist.next_win_ready = false;
	local_persist.next_window    = nullptr;
}

///////////////////////////////////////////

void android_set_window(void *window) {
	local_persist.next_window    = (ANativeWindow*)window;
	local_persist.next_win_ready = true;
}

///////////////////////////////////////////

void android_set_window_xam(void *window) {
	local_persist.next_window_xam = (jobject)window;
	local_persist.next_win_ready  = true;
}

///////////////////////////////////////////

ANativeWindow* android_get_window() {
	return local.window;
}

///////////////////////////////////////////

bool android_read_asset(const char* asset_name, void** out_data, size_t* out_size) {
	// See: http://www.50ply.com/blog/2013/01/19/loading-compressed-android-assets-with-file-pointer/
	AAsset *asset = AAssetManager_open(local.asset_manager, asset_name, AASSET_MODE_BUFFER);
	if (asset) {
		*out_size = AAsset_getLength(asset);
		*out_data = sk_malloc(*out_size + 1);
		AAsset_read (asset, *out_data, *out_size);
		AAsset_close(asset);

		((uint8_t *)*out_data)[*out_size] = 0;
		return true;
	}
	return false;
}

///////////////////////////////////////////
// Backend                               //
///////////////////////////////////////////

void *backend_android_get_java_vm () { return local_persist.vm; }
void *backend_android_get_activity() { return local.activity; }
void *backend_android_get_jni_env () { return local.env; }

///////////////////////////////////////////

void platform_msgbox_err(const char* text, const char* header) {
	log_warn("No messagebox capability for this platform!");
}

///////////////////////////////////////////

font_t platform_default_font() {
	// If we're using Android API 29+, we can just look up the system font!
	array_t<const char *> fonts         = array_t<const char *>::make(2);
	font_t                result        = nullptr;
	const char           *file_latin    = nullptr;
	const char           *file_japanese = nullptr;

#if __ANDROID_API__ >= 29
	AFontMatcher *matcher = AFontMatcher_create();
	AFont *font_latin    = AFontMatcher_match(matcher, "sans-serif", (uint16_t*)u"A", 1, nullptr);
	AFont *font_japanese = AFontMatcher_match(matcher, "sans-serif", (uint16_t*)u"\u3042", 1, nullptr);
	if (font_latin   ) file_latin    = AFont_getFontFilePath(font_latin);
	if (font_japanese) file_japanese = AFont_getFontFilePath(font_japanese);
#endif
	if      (file_latin != nullptr)                                      fonts.add(file_latin);
	else if (ska_file_exists("/system/fonts/NotoSans-Regular.ttf")) fonts.add("/system/fonts/NotoSans-Regular.ttf");
	else if (ska_file_exists("/system/fonts/Roboto-Regular.ttf"  )) fonts.add("/system/fonts/Roboto-Regular.ttf");
	else if (ska_file_exists("/system/fonts/DroidSans.ttf"       )) fonts.add("/system/fonts/DroidSans.ttf");

	if      (file_japanese != nullptr)                                      fonts.add(file_japanese);
	else if (ska_file_exists("/system/fonts/NotoSansCJK-Regular.ttc")) fonts.add("/system/fonts/NotoSansCJK-Regular.ttc");
	else if (ska_file_exists("/system/fonts/DroidSansJapanese.ttf"  )) fonts.add("/system/fonts/DroidSansJapanese.ttf");

	if (fonts.count > 0)
		result = font_create_files(fonts.data, fonts.count);

#if __ANDROID_API__ >= 29
	if (font_latin   ) AFont_close(font_latin);
	if (font_japanese) AFont_close(font_japanese);
	AFontMatcher_destroy(matcher);
#endif
	fonts.free();
	return result;
}

///////////////////////////////////////////

#include <unwind.h>
#include <dlfcn.h>
#include <cxxabi.h>

struct android_backtrace_state {
	void **current;
	void **end;
};

void platform_print_callstack() {
	const int max = 100;
	void* buffer[max];

	android_backtrace_state state;
	state.current = buffer;
	state.end = buffer + max;

	_Unwind_Backtrace([](struct _Unwind_Context* context, void* arg) {
		android_backtrace_state* state = (android_backtrace_state *)arg;
		uintptr_t pc = _Unwind_GetIP(context);
		if (pc) {
			if (state->current == state->end)
				return (_Unwind_Reason_Code)_URC_END_OF_STACK;
			else
				*state->current++ = reinterpret_cast<void*>(pc);
		}
		return (_Unwind_Reason_Code)_URC_NO_REASON;
	}, &state);

	int count = (int)(state.current - buffer);

	for (int idx = 0; idx < count; idx++)  {
		const void* addr   = buffer[idx];
		const char* symbol = "";

		Dl_info info;
		if (dladdr(addr, &info) && info.dli_sname)
			symbol = info.dli_sname;
		int   status    = 0;
		char *demangled = __cxxabiv1::__cxa_demangle(symbol, 0, 0, &status);

		sk::log_diagf("%03d: 0x%p %s", idx, addr,
			(nullptr != demangled && 0 == status) ?
			demangled : symbol);

		sk_free(demangled);
	}
}

} // namespace sk

#endif
