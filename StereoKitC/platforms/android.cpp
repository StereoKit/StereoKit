#include "android.h"
#if defined(SK_OS_ANDROID)

#include "../log.h"
#include "../device.h"
#include "../sk_math.h"
#include "../xr_backends/openxr.h"
#include "../systems/render.h"
#include "../systems/system.h"
#include "../_stereokit.h"
#include "../libraries/sk_gpu.h"
#include "../libraries/sokol_time.h"

#include <android/native_activity.h>
#include <android/native_window_jni.h>
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>

#include <dlfcn.h>

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

// When consumed as a static library, the user's app will need to provide its
// own JNI_OnLoad function, which will then conflict with this one. So we only
// provide this for shared libraries.
#if defined(SK_BUILD_SHARED)

extern "C" jint JNI_OnLoad(JavaVM* vm, void* reserved) {
	android_vm = vm;
	return JNI_VERSION_1_6;
}
extern "C" jint JNI_OnLoad_L(JavaVM* vm, void* reserved) {
	return JNI_OnLoad(vm, reserved);
}

#endif

///////////////////////////////////////////

bool android_init() {
	// don't allow flatscreen fallback on Android
	sk_settings.no_flatscreen_fallback = true;

	android_render_sys = systems_find("FrameRender");
	android_activity   = (jobject)sk_settings.android_activity;
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
	jclass    activity_class = android_env->GetObjectClass(android_activity);
	jmethodID activity_class_getAssets = android_env->GetMethodID(activity_class, "getAssets", "()Landroid/content/res/AssetManager;");
	jobject   asset_manager            = android_env->CallObjectMethod(android_activity, activity_class_getAssets); // activity.getAssets();
	jobject   global_asset_manager     = android_env->NewGlobalRef(asset_manager);
	android_asset_manager = AAssetManager_fromJava(android_env, global_asset_manager);
	if (android_asset_manager == nullptr) {
		log_fail_reason(95, log_error, "Couldn't get the Android asset manager!");
		return false;
	}

#if defined(SK_DYNAMIC_OPENXR)
	// Android has no universally supported openxr_loader yet, so on this
	// platform we don't static link it, and instead provide devs a way to ship
	// other loaders.
	if (sk_settings.display_preference == display_mode_mixedreality && dlopen("libopenxr_loader.so", RTLD_NOW) == nullptr) {
		log_fail_reason(95, log_error, "openxr_loader failed to load!");
		return false;
	}
#endif

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
	device_data.display_width  = android_swapchain.width;
	device_data.display_height = android_swapchain.height;
	render_update_projection();
	log_diagf("Created swapchain: %dx%d color:%s depth:%s", android_swapchain.width, android_swapchain.height, render_fmt_name((tex_format_)color_fmt), render_fmt_name((tex_format_)depth_fmt));
}

///////////////////////////////////////////

void android_resize_swapchain() {
	int32_t height = maxi(1,ANativeWindow_getWidth (android_window));
	int32_t width  = maxi(1,ANativeWindow_getHeight(android_window));

	if (!android_swapchain_created || (width == sk_info.display_width && height == sk_info.display_height))
		return;

	log_diagf("Resized swapchain: %dx%d", width, height);
	skg_swapchain_resize(&android_swapchain, width, height);
	sk_info.display_width  = width;
	sk_info.display_height = height;
	device_data.display_width  = width;
	device_data.display_height = height;
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
	sk_info.display_type      = display_opaque;
	device_data.display_blend = display_blend_opaque;
	if (android_window) {
		android_create_swapchain();
	}
	return true;
}

///////////////////////////////////////////

void android_stop_flat() {
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
				if (device_data.display_type == display_type_flatscreen)
					android_create_swapchain();
			}
		}
		android_next_win_ready = false;
		android_next_window    = nullptr;
	}
}

///////////////////////////////////////////

void android_step_end_flat() {
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

const int32_t PERMISSION_DENIED  = -1;
const int32_t PERMISSION_GRANTED = 0;

///////////////////////////////////////////

bool android_check_app_permission(const char* permission) {
	jclass    class_activity               = android_env->GetObjectClass(android_activity);
	jmethodID activity_checkSelfPermission = android_env->GetMethodID   (class_activity, "checkSelfPermission", "(Ljava/lang/String;)I");

	jstring jobj_permission = android_env->NewStringUTF(permission);
	jint    result          = android_env->CallIntMethod(android_activity, activity_checkSelfPermission, jobj_permission);

	android_env->DeleteLocalRef(jobj_permission);

	return result == PERMISSION_GRANTED;
}

///////////////////////////////////////////

bool android_check_manifest_permission(const char* permission)
{
	jclass       class_activity                =          android_env->GetObjectClass  (android_activity);
	jmethodID    activity_getPackageManager    =          android_env->GetMethodID     (class_activity, "getPackageManager", "()Landroid/content/pm/PackageManager;");
	jmethodID    activity_getPackageName       =          android_env->GetMethodID     (class_activity, "getPackageName", "()Ljava/lang/String;");
	jobject      jobj_packageManager           =          android_env->CallObjectMethod(android_activity, activity_getPackageManager);
	jclass       class_packageManager          =          android_env->GetObjectClass  (jobj_packageManager);
	jmethodID    packageManager_getPackageInfo =          android_env->GetMethodID     (class_packageManager, "getPackageInfo", "(Ljava/lang/String;I)Landroid/content/pm/PackageInfo;");
	jstring      jobj_packageName              = (jstring)android_env->CallObjectMethod(android_activity, activity_getPackageName);

	jint         flags = 4096; // PackageManager.GET_PERMISSIONS
	jobject      jobj_packageInfo  = android_env->CallObjectMethod(jobj_packageManager, packageManager_getPackageInfo, jobj_packageName, flags);
	jclass       class_packageInfo = android_env->GetObjectClass(jobj_packageInfo);

	jfieldID     packageInfo_requestedPermissions = android_env->GetFieldID(class_packageInfo, "requestedPermissions", "[Ljava/lang/String;");
	jobjectArray jobj_requestedPermissions        = (jobjectArray)android_env->GetObjectField(jobj_packageInfo, packageInfo_requestedPermissions);

	jstring      jobj_permission = android_env->NewStringUTF(permission);
	jclass       class_string    = android_env->GetObjectClass(jobj_permission);
	jmethodID    string_equals   = android_env->GetMethodID(class_string, "equals", "(Ljava/lang/Object;)Z");

	// Iterate over the array
	jsize length = android_env->GetArrayLength(jobj_requestedPermissions);
	bool  result = false;
	for (jsize i = 0; i < length; i++) {
		jstring jobj_currPermission = (jstring)android_env->GetObjectArrayElement(jobj_requestedPermissions, i);
		bool match = android_env->CallBooleanMethod(jobj_permission, string_equals, jobj_currPermission);
		android_env->DeleteLocalRef(jobj_currPermission);

		if (match) {
			result = true;
			break;
		}
	}

	android_env->DeleteLocalRef(jobj_packageManager);
	android_env->DeleteLocalRef(jobj_packageName);
	android_env->DeleteLocalRef(jobj_packageInfo);
	android_env->DeleteLocalRef(jobj_requestedPermissions);
	android_env->DeleteLocalRef(jobj_permission);

	return result;

}

///////////////////////////////////////////

void android_request_permission(const char* permission) {
	jclass    class_activity                    = android_env->GetObjectClass(android_activity);
	jmethodID contextCompat_checkSelfPermission = android_env->GetMethodID   (class_activity, "checkSelfPermission", "(Ljava/lang/String;)I");
	jmethodID activity_requestPermissions       = android_env->GetMethodID   (class_activity, "requestPermissions",  "([Ljava/lang/String;I)V");

	jstring      jobj_permission      = android_env->NewStringUTF  (permission);
	jobjectArray jobj_permission_list = android_env->NewObjectArray(1, android_env->FindClass("java/lang/String"), NULL);

	android_env->SetObjectArrayElement(jobj_permission_list, 0, jobj_permission);
	android_env->CallVoidMethod       (android_activity, activity_requestPermissions, jobj_permission_list, 0);

	android_env->DeleteLocalRef(jobj_permission);
	android_env->DeleteLocalRef(jobj_permission_list);
}

///////////////////////////////////////////

void *backend_android_get_java_vm () { return android_vm; }
void *backend_android_get_activity() { return android_activity; }
void *backend_android_get_jni_env () { return android_env; }

} // namespace sk

#endif