#include "android.h"
#if defined(SK_OS_ANDROID)

#include "../log.h"
#include "../xr_backends/openxr.h"
#include "flatscreen_input.h"
#include "../systems/render.h"
#include "../systems/input.h"
#include "../systems/system.h"
#include "../_stereokit.h"
#include "../libraries/sk_gpu.h"
#include "../libraries/sokol_time.h"

#include <android/native_activity.h>
#include <android/native_window_jni.h>
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>

namespace sk {

JavaVM           *android_vm              = nullptr;
jobject           android_activity        = nullptr;
JNIEnv           *android_env             = nullptr;
AAssetManager    *android_asset_manager   = nullptr;
ANativeWindow    *android_window          = nullptr;
ANativeWindow    *android_next_window     = nullptr;
jobject           android_next_window_xam = nullptr;
bool              android_next_win_ready  = false;
skg_swapchain_t   android_swapchain       = {};
bool              android_swapchain_created = false;
system_t         *android_render_sys = nullptr;

///////////////////////////////////////////

extern "C" jint JNI_OnLoad(JavaVM* vm, void* reserved) {
	android_vm = vm;
	return JNI_VERSION_1_6;
}
extern "C" jint JNI_OnLoad_L(JavaVM* vm, void* reserved) {
	return JNI_OnLoad(vm, reserved);
}

///////////////////////////////////////////

bool android_init() {
	android_render_sys = systems_find("FrameRender");
	android_activity = (jobject)sk_settings.android_activity;
	if (android_vm == nullptr)
		android_vm = (JavaVM*)sk_settings.android_java_vm;

	if (android_vm == nullptr || android_activity == nullptr) {
		log_fail_reason(95, log_error, "Couldn't find Android's Java VM or Activity, you should load the StereoKitC library with something like Xamarin's JavaSystem.LoadLibrary, or manually assign it using sk_set_settings()");
		return false;
	}

	// Get the java environment from the VM, and ensure it's attached to this
	// thread
	int result = android_vm->GetEnv((void **)&android_env, JNI_VERSION_1_6);
	if (result == JNI_EDETACHED) {
		if (android_vm->AttachCurrentThread(&android_env, nullptr) != JNI_OK) {
			log_fail_reason(95, log_error, "Couldn't attach the Java Environment to the current thread!");
			return false;
		}
	} else if (result != JNI_OK) {
		log_fail_reason(95, log_error, "Couldn't get the Java Environment from the VM, this needs to be called from the main thread.");
		return false;
	}

	// https://stackoverflow.com/questions/51099200/native-crash-jni-detected-error-in-application-thread-using-jnienv-from-th
	//android_vm->AttachCurrentThread(&android_env, nullptr);

	// Find the current android activity
	// https://stackoverflow.com/questions/46869901/how-to-get-the-android-context-instance-when-calling-jni-method
	/*jclass    activity_thread      = android_env->FindClass("android/app/ActivityThread");
	jmethodID curr_activity_thread = android_env->GetStaticMethodID(activity_thread, "currentActivityThread", "()Landroid/app/ActivityThread;");
	jobject   at                   = android_env->CallStaticObjectMethod(activity_thread, curr_activity_thread);
	jmethodID get_application      = android_env->GetMethodID(activity_thread, "getApplication", "()Landroid/app/Application;");
	jobject   activity_inst        = android_env->CallObjectMethod(at, get_application);
	android_activity = android_env->NewGlobalRef(activity_inst);
	if (android_activity == nullptr) {
		log_fail_reason(95,  "Couldn't find the current Android application context!");
		return false;
	}*/

	// Get the asset manager for loading files
	// from https://stackoverflow.com/questions/22436259/android-ndk-why-is-aassetmanager-open-returning-null/22436260#22436260
	jclass    activity_class           = android_env->GetObjectClass(android_activity);
	jmethodID activity_class_getAssets = android_env->GetMethodID(activity_class, "getAssets", "()Landroid/content/res/AssetManager;");
	jobject   asset_manager            = android_env->CallObjectMethod(android_activity, activity_class_getAssets); // activity.getAssets();
	jobject   global_asset_manager     = android_env->NewGlobalRef(asset_manager);
	android_asset_manager = AAssetManager_fromJava(android_env, global_asset_manager);
	if (android_asset_manager == nullptr) {
		log_fail_reason(95, log_error, "Couldn't get the Android asset manager!");
		return false;
	}

	return true;
}

///////////////////////////////////////////

void android_create_swapchain() {
	skg_tex_fmt_ color_fmt = skg_tex_fmt_rgba32_linear;
	skg_tex_fmt_ depth_fmt = (skg_tex_fmt_)render_preferred_depth_fmt();
	android_swapchain = skg_swapchain_create(android_window, color_fmt, depth_fmt, sk_info.display_width, sk_info.display_height);
	android_swapchain_created = true;
	sk_info.display_width  = android_swapchain.width;
	sk_info.display_height = android_swapchain.height;
	render_update_projection();
	log_diagf("Created swapchain: %dx%d color:%s depth:%s", android_swapchain.width, android_swapchain.height, render_fmt_name((tex_format_)color_fmt), render_fmt_name((tex_format_)depth_fmt));
}

///////////////////////////////////////////

void android_resize_swapchain() {
	int32_t height = ANativeWindow_getWidth (android_window);
	int32_t width  = ANativeWindow_getHeight(android_window);

	if (!android_swapchain_created || (width == sk_info.display_width && height == sk_info.display_height))
		return;

	log_diagf("Resized swapchain: %dx%d", width, height);
	skg_swapchain_resize(&android_swapchain, width, height);
	sk_info.display_width  = width;
	sk_info.display_height = height;
	render_update_projection();
}

///////////////////////////////////////////

void android_set_window(void *window) {
	android_next_window    = (ANativeWindow*)window;
	android_next_win_ready = true;
}

///////////////////////////////////////////

void android_set_window_xam(void *window) {
	android_next_window_xam = (jobject)window;
	android_next_win_ready  = true;
}

///////////////////////////////////////////

bool android_start_pre_xr() {
	return true;
}

///////////////////////////////////////////

bool android_start_post_xr() {
	return true;
}

///////////////////////////////////////////

bool android_start_flat() {
	if (android_window) {
		android_create_swapchain();
	}
	flatscreen_input_init();
	return true;
}

///////////////////////////////////////////

void android_stop_flat() {
	flatscreen_input_shutdown();
	if (android_window) {
		android_swapchain_created = false;
		skg_swapchain_destroy(&android_swapchain);
		android_window = nullptr;
	}
}

///////////////////////////////////////////

void android_shutdown() {
	if (android_vm)
		android_vm->DetachCurrentThread();
}

///////////////////////////////////////////

void android_step_begin_xr() {
}

///////////////////////////////////////////

void android_step_begin_flat() {
	flatscreen_input_update();

	if (android_next_win_ready) {
		// If we got our window from xamarin, it's a jobject, and needs
		// converted into an ANativeWindow first!
		if (android_next_window_xam != nullptr) {
			android_next_window = ANativeWindow_fromSurface(android_env, android_next_window_xam);
			android_next_window_xam = nullptr;
		}

		if (android_window != nullptr && android_window == android_next_window) {
			// It's the same window, lets just resize it
			android_resize_swapchain();
		} else {
			// Completely new window! Destroy the old swapchain, and make a 
			// new one.
			if (android_window != nullptr && android_next_window != nullptr)
				skg_swapchain_destroy(&android_swapchain);

			if (android_next_window) {
				android_window = (ANativeWindow*)android_next_window;
				if (sk_display_mode == display_mode_flatscreen)
					android_create_swapchain();
			}
		}
		android_next_win_ready = false;
		android_next_window    = nullptr;
	}
}

///////////////////////////////////////////

void android_step_end_flat() {
	input_update_poses(true);

	if (!android_window)
		return;

	skg_draw_begin();
	color128 color = render_get_clear_color_ln();
	skg_swapchain_bind(&android_swapchain);
	skg_target_clear(true, &color.r);

	matrix view = render_get_cam_final        ();
	matrix proj = render_get_projection_matrix();
	matrix_inverse(view, view);
	render_draw_matrix(&view, &proj, 1, render_get_filter());
	render_clear();
	
	android_render_sys->profile_frame_duration = stm_since(android_render_sys->profile_frame_start);
	skg_swapchain_present(&android_swapchain);
}

///////////////////////////////////////////

void *backend_android_get_java_vm () { return android_vm; }
void *backend_android_get_activity() { return android_activity; }
void *backend_android_get_jni_env () { return android_env; }

} // namespace sk

#endif