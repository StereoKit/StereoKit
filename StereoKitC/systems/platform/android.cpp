#ifdef __ANDROID__

#include "../../log.h"
#include "android.h"
#include "openxr.h"
#include "flatscreen_input.h"
#include "../render.h"
#include "../input.h"
#include "../../_stereokit.h"
#include "../../libraries/sk_gpu.h"

#include <android/native_activity.h>
#include <android/native_window_jni.h>
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>

namespace sk {

JavaVM           *android_vm       = nullptr;
JNIEnv           *android_env      = nullptr;
jobject           android_activity = nullptr;
ANativeWindow    *android_window   = nullptr;
AAssetManager    *android_asset_manager = nullptr;
skg_swapchain_t   android_swapchain;

///////////////////////////////////////////

extern "C" jint JNI_OnLoad(JavaVM* vm, void* reserved) {
	android_vm = vm;

	return JNI_VERSION_1_6;
}
extern "C" jint JNI_OnLoad_L(JavaVM* vm, void* reserved) {
	return JNI_OnLoad(vm, reserved);
}

///////////////////////////////////////////

bool android_setup(void *from_window) {
	if (android_vm == nullptr) {
		if (from_window == nullptr) {
			log_fail_reason(95,  "sk_android_info_t wasn't provided!");
			return false;
		}

		sk_android_info_t *android_info = (sk_android_info_t*)from_window;
		android_vm            = (JavaVM*)android_info->java_vm;
		android_env           = (JNIEnv*)android_info->jni_env;
		android_activity      = (jobject)android_info->activity;
		android_asset_manager = (AAssetManager *)android_info->asset_manager;
		android_window        = (ANativeWindow *)android_info->window;
		if (android_vm == nullptr || android_env == nullptr || android_activity == nullptr ||
			android_asset_manager == nullptr || android_window == nullptr) {
			log_fail_reason(95,  "sk_android_info_t wasn't filled out entirely!");
			return false;
		} else return true;
	}

	if (android_vm == nullptr) {
		log_fail_reason(95,  "Couldn't find the Java VM, you should manually load the StereoKitC library with something like Xamarin's JavaSystem.LoadLibrary");
		return false;
	}

	// Get the java environment from the VM
	if (android_vm->GetEnv((void **)&android_env, JNI_VERSION_1_6) != JNI_OK) {
		log_fail_reason(95, "Couldn't get the Java Environment from the VM, this needs to be called from the main thread.");
		return false;
	}

	// https://stackoverflow.com/questions/51099200/native-crash-jni-detected-error-in-application-thread-using-jnienv-from-th
	android_vm->AttachCurrentThread(&android_env, nullptr);

	struct xam_init_data_t {
		jobject activity;
		jobject window;
	};
	xam_init_data_t *info = (xam_init_data_t*)from_window;
	android_activity = info->activity;// android_env->NewGlobalRef(info->activity);

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
		log_fail_reason(95,  "Couldn't get the asset manager!");
		return false;
	}

	// Find the window surface
	android_window = ANativeWindow_fromSurface(android_env, info->window);

	return true;
}

///////////////////////////////////////////

void android_create_window() {
	return true;
}

///////////////////////////////////////////

bool android_init() {
	skg_callback_log([](skg_log_ level, const char *text) {
		switch (level) {
		case skg_log_info:     log_diagf("sk_gpu: %s", text); break;
		case skg_log_warning:  log_warnf("sk_gpu: %s", text); break;
		case skg_log_critical: log_errf ("sk_gpu: %s", text); break;
		}
	});
	if (!skg_init(sk_app_name, android_window, nullptr))
		return false;
	log_diag("sk_gpu initialized!");

	skg_tex_fmt_ color_fmt = skg_tex_fmt_rgba32_linear;
	skg_tex_fmt_ depth_fmt = skg_tex_fmt_depth16;
	android_swapchain = skg_swapchain_create(color_fmt, depth_fmt, sk_info.display_width, sk_info.display_height);
	sk_info.display_width  = android_swapchain.width;
	sk_info.display_height = android_swapchain.height;
	log_diagf("Created swapchain: %dx%d color:%s depth:%s", android_swapchain.width, android_swapchain.height, render_fmt_name((tex_format_)color_fmt), render_fmt_name((tex_format_)depth_fmt));

	flatscreen_input_init();
	return true;
}

///////////////////////////////////////////

void android_shutdown() {
	flatscreen_input_shutdown();
	skg_swapchain_destroy(&android_swapchain);
	skg_shutdown();

	if (android_vm)
		android_vm->DetachCurrentThread();
}

///////////////////////////////////////////

void android_step_begin() {
	flatscreen_input_update();
}

///////////////////////////////////////////

void android_step_end() {
	skg_draw_begin();

	color128   col    = render_get_clear_color();
	skg_tex_t *target = skg_swapchain_get_next(&android_swapchain);
	skg_tex_target_bind(target, true, &col.r);

	input_update_predicted();

	matrix view = render_get_cam_root  ();
	matrix proj = render_get_projection();
	matrix_inverse(view, view);
	render_draw_matrix(&view, &proj, 1);
	render_clear();
}

///////////////////////////////////////////

void android_vsync() {
	skg_swapchain_present(&android_swapchain);
}

} // namespace sk

#endif