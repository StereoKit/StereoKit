// SPDX-License-Identifier: MIT
// The authors below grant copyright rights under the MIT license:
// Copyright (c) 2025 Nick Klingensmith
// Copyright (c) 2025 Qualcomm Technologies, Inc.

#include "permission.h"
#include "../platforms/platform.h"
#include "../libraries/stref.h"
#include "../sk_memory.h"

namespace sk {

///////////////////////////////////////////

#if defined(SK_OS_ANDROID)

///////////////////////////////////////////

#include <android/native_activity.h>

enum xr_runtime_ {
	xr_runtime_none,
	xr_runtime_unknown,
	xr_runtime_meta,
	xr_runtime_android_xr,
	xr_runtime_vive,
	xr_runtime_pico,
	xr_runtime_monado,
};

struct permission_state_t {
	permission_state_ present             [permission_type_max];
	bool              requires_interaction[permission_type_max];
	const char*       permission_str      [permission_type_max];

	jclass    class_activity;
	jmethodID activity_checkSelfPermission;
	jmethodID activity_requestPermissions;
};
static permission_state_t local;

const int32_t PERMISSION_DENIED = -1;
const int32_t PERMISSION_GRANTED = 0;
const int32_t PERMISSION_PROTECTION_DANGEROUS = 0x1;
const int32_t PROTECTION_MASK_BASE = 0x0000000f;

///////////////////////////////////////////

bool        _permission_check_app_permission      (const char* permission);
void        _permission_check_manifest_permissions(void);
void        _permission_request_permission        (const char* permission);
xr_runtime_ _permission_check_runtime             (void);
bool        _permission_manifest_has              (JNIEnv* env, jobjectArray permission_list, jsize list_count, jmethodID string_equals, const char* permission_str);
const char* _permission_check_string              (permission_type_ type, xr_runtime_ runtime, JNIEnv* env, jobjectArray permission_list, jsize list_count, jmethodID string_equals);

///////////////////////////////////////////

const char* _permission_check_string(permission_type_ type, xr_runtime_ runtime, JNIEnv* env, jobjectArray permission_list, jsize list_count, jmethodID string_equals) {
	const char* result = nullptr;

	#define PERMISSION_CHECK(r, s) if (runtime == r && !result && _permission_manifest_has(env, permission_list, list_count, string_equals, s)) { result = s; }
	#define PERMISSION_SET(r, s) if (runtime == r && !result) { result = s; }

	// Some good sources for finding permission strings
	// - Android XR
	//   https://developer.android.com/develop/xr/openxr/extensions
	//   https://developer.android.com/develop/xr/get-started#understand-permissions
	// - Android - Standard, non-xr permissions
	//   https://developer.android.com/reference/android/Manifest.permission
	// - Meta
	//   https://github.com/search?q=repo%3Ameta-quest%2FMeta-OpenXR-SDK+com.oculus.permission&type=code
	// - Pico
	//   https://github.com/search?q=owner%3Apicoxr%20com.picovr.permission&type=code
	// - Vive - Doesn't seem to use any permissions (2025.9)?
	//   https://github.com/search?q=org%3AViveSoftware+uses-permission&type=code
	// - Monado - Doesn't _exactly_ have its own Android runtime
	//   https://gitlab.freedesktop.org/search?search=uses-permission&project_id=2685&group_id=5604&search_code=true
	// - Godot - misc permissions
	//   https://github.com/search?q=org%3AGodotVR%20uses-permission&type=code
	
	// This is an incomplete list of per-runtime permission strings.
	// If you're adding new permissions to SK's list, this plus the
	// permission_type_ enum in stereokit.h is where you'll add it!
	switch (type) {

	// These are standardized permissions for AOSP, so no need to dance around
	case permission_type_microphone: result = "android.permission.RECORD_AUDIO"; break;
	case permission_type_camera:     result = "android.permission.CAMERA";       break;

	// These permissions are different on every AOSP OpenXR runtime, some
	// runtimes like Meta even have multiple official strings. Meta's
	// "com.oculus" strings are deprecated, and the "horizonos.permission"
	// strings aren't documented yet. We still should support them both,
	// since StereoKit doesn't control what's in the AndroidManifest.xml.
	case permission_type_eye_input:
		PERMISSION_SET  (xr_runtime_android_xr, "android.permission.EYE_TRACKING_FINE");
		PERMISSION_CHECK(xr_runtime_meta,       "com.oculus.permission.EYE_TRACKING");
		PERMISSION_SET  (xr_runtime_meta,       "horizonos.permission.EYE_TRACKING");
		PERMISSION_SET  (xr_runtime_pico,       "com.picovr.permission.EYE_TRACKING");
		break;
	case permission_type_hand_tracking:
		PERMISSION_SET  (xr_runtime_android_xr, "android.permission.HAND_TRACKING");
		PERMISSION_CHECK(xr_runtime_meta,       "com.oculus.permission.HAND_TRACKING");
		PERMISSION_SET  (xr_runtime_meta,       "horizonos.permission.HAND_TRACKING");
		PERMISSION_SET  (xr_runtime_pico,       "com.picovr.permission.HAND_TRACKING");
		break;
	case permission_type_face_tracking:
		PERMISSION_SET  (xr_runtime_android_xr, "android.permission.FACE_TRACKING");
		PERMISSION_CHECK(xr_runtime_meta,       "com.oculus.permission.FACE_TRACKING");
		PERMISSION_SET  (xr_runtime_meta,       "horizonos.permission.FACE_TRACKING");
		PERMISSION_SET  (xr_runtime_pico,       "com.picovr.permission.FACE_TRACKING");
		break;
	case permission_type_scene:
		PERMISSION_SET  (xr_runtime_android_xr, "android.permission.SCENE_UNDERSTANDING_COARSE");
		PERMISSION_CHECK(xr_runtime_meta,       "com.oculus.permission.USE_SCENE");
		PERMISSION_SET  (xr_runtime_meta,       "horizonos.permission.USE_SCENE");
		PERMISSION_SET  (xr_runtime_pico,       "com.picovr.permission.SPATIAL_DATA");
		break;
	default:
		break;
	}

	#undef PERMISSION_CHECK
	#undef PERMISSION_SET

	return result;
}

///////////////////////////////////////////

xr_runtime_ _permission_check_runtime() {
	const char* runtime_name = device_get_runtime();
	xr_runtime_ result       = xr_runtime_unknown;

	// This is an incomplete list of runtimes
	if      (string_startswith(runtime_name, "Meta"  )) result = xr_runtime_meta;
	else if (string_startswith(runtime_name, "Oculus")) result = xr_runtime_meta;
	else if (string_startswith(runtime_name, "Monado")) result = xr_runtime_monado;
	else if (string_startswith(runtime_name, "Moohan")) result = xr_runtime_android_xr;
	else if (string_startswith(runtime_name, "Vive"  )) result = xr_runtime_vive;

	return result;
}

///////////////////////////////////////////

bool permission_init() {
	local = {};

	// Cache a few JNI objects for faster permission checking, this is used by
	// android_check_app_permission, which may be used every frame.
	JNIEnv* env      = (JNIEnv*)backend_android_get_jni_env();
	jobject activity = (jobject)backend_android_get_activity();

	jclass local_class_activity = env->GetObjectClass(activity);
	// Promote this one to a global reference, so it doesn't get cleaned up
	local.class_activity = (jclass)env->NewGlobalRef(local_class_activity);
	env->DeleteLocalRef(local_class_activity);

	// A jmethodID is valid for as long as the class is, no global ref needed
	local.activity_checkSelfPermission = env->GetMethodID(local.class_activity, "checkSelfPermission", "(Ljava/lang/String;)I");
	local.activity_requestPermissions  = env->GetMethodID(local.class_activity, "requestPermissions", "([Ljava/lang/String;I)V");


	// Check what permissions we've got in the manifest
	_permission_check_manifest_permissions();

	return true;
}

///////////////////////////////////////////

void permission_shutdown() {
	JNIEnv* env = (JNIEnv*)backend_android_get_jni_env();
	env->DeleteGlobalRef(local.class_activity);

	local = {};
}

///////////////////////////////////////////

bool32_t permission_is_interactive(permission_type_ permission) {
	return local.requires_interaction[permission];
}

///////////////////////////////////////////

permission_state_ permission_state(permission_type_ permission) {
	if (local.present[permission] == permission_state_capable) {
		if (_permission_check_app_permission(local.permission_str[permission]))
			local.present[permission] = permission_state_granted;
	}
	return local.present[permission];
}

///////////////////////////////////////////

void permission_request(permission_type_ permission) {
	if (local.permission_str[permission] == NULL) {
		log_warnf("Permission string for 0x%X unknown on current platform", permission);
		return;
	}

	_permission_request_permission(local.permission_str[permission]);
}

///////////////////////////////////////////

bool _permission_check_app_permission(const char* permission) {
	JNIEnv* env      = (JNIEnv*)backend_android_get_jni_env();
	jobject activity = (jobject)backend_android_get_activity();

	jstring jobj_permission = env->NewStringUTF(permission);
	jint    result          = env->CallIntMethod(activity, local.activity_checkSelfPermission, jobj_permission);
	env->DeleteLocalRef(jobj_permission);

	return result == PERMISSION_GRANTED;
}

///////////////////////////////////////////

bool _permission_manifest_has(JNIEnv* env, jobjectArray permission_list, jsize list_count, jmethodID string_equals, const char* permission_str) {
	bool result = false;
	jstring jobj_permission_str = env->NewStringUTF(permission_str);
	for (jsize i = 0; i < list_count; i++) {
		jstring jobj_curr_permission = (jstring)env->GetObjectArrayElement(permission_list, i);

		result = env->CallBooleanMethod(jobj_permission_str, string_equals, jobj_curr_permission);

		env->DeleteLocalRef(jobj_curr_permission);

		if (result) break;
	}
	env->DeleteLocalRef(jobj_permission_str);
	return result;
}

///////////////////////////////////////////

void _permission_check_manifest_permissions() {
	JNIEnv* env      = (JNIEnv*)backend_android_get_jni_env ();
	jobject activity = (jobject)backend_android_get_activity();

	jmethodID    activity_getPackageManager    =          env->GetMethodID     (local.class_activity, "getPackageManager", "()Landroid/content/pm/PackageManager;");
	jmethodID    activity_getPackageName       =          env->GetMethodID     (local.class_activity, "getPackageName", "()Ljava/lang/String;");
	jmethodID    activity_shouldShowRationale  =          env->GetMethodID     (local.class_activity, "shouldShowRequestPermissionRationale", "(Ljava/lang/String;)Z");
	jobject      jobj_packageManager           =          env->CallObjectMethod(activity, activity_getPackageManager);
	jclass       class_packageManager          =          env->GetObjectClass  (jobj_packageManager);
	jmethodID    packageManager_getPackageInfo =          env->GetMethodID     (class_packageManager, "getPackageInfo",    "(Ljava/lang/String;I)Landroid/content/pm/PackageInfo;");
	jmethodID    packageManager_getPermissionInfo =       env->GetMethodID     (class_packageManager, "getPermissionInfo", "(Ljava/lang/String;I)Landroid/content/pm/PermissionInfo;");

	jstring      jobj_packageName              = (jstring)env->CallObjectMethod(activity, activity_getPackageName);

	jint         flags = 4096; // PackageManager.GET_PERMISSIONS
	jobject      jobj_packageInfo  = env->CallObjectMethod(jobj_packageManager, packageManager_getPackageInfo, jobj_packageName, flags);
	jclass       class_packageInfo = env->GetObjectClass  (jobj_packageInfo);

	jfieldID     packageInfo_requestedPermissions =               env->GetFieldID    (class_packageInfo, "requestedPermissions", "[Ljava/lang/String;");
	jobjectArray jobj_requestedPermissions        = (jobjectArray)env->GetObjectField(jobj_packageInfo, packageInfo_requestedPermissions);

	jclass    class_string  = env->FindClass  ("java/lang/String");
	jmethodID string_equals = env->GetMethodID(class_string, "equals", "(Ljava/lang/Object;)Z");

	// look through all permissions in the manifest, and match them up to ones
	// StereoKit knows about.
	jsize       length  = jobj_requestedPermissions ? env->GetArrayLength(jobj_requestedPermissions) : 0;
	xr_runtime_ runtime = _permission_check_runtime();
	for (int32_t p = 0; p < permission_type_max; p++) {

		local.permission_str[p] = _permission_check_string((permission_type_)p, runtime, env, jobj_requestedPermissions, length, string_equals);

		if (local.permission_str[p] == nullptr) {
			// We didn't find a string we know about for this runtime, we have
			// no idea about this permission.
			local.present[p] = permission_state_unknown;
			continue;
		}
		if (!_permission_manifest_has(env, jobj_requestedPermissions, length, string_equals, local.permission_str[p])) {
			// We know the permission string, but it is not present in the
			// app's manifest, so we can't use it.
			local.present[p] = permission_state_unavailable;
			continue;
		}

		jstring  jstr_permission = env->NewStringUTF(local.permission_str[p]);
		jboolean is_interactive  = env->CallBooleanMethod(activity, activity_shouldShowRationale, jstr_permission);
		jobject  permission_info = env->CallObjectMethod(jobj_packageManager, packageManager_getPermissionInfo, jstr_permission, 0);
		if (env->ExceptionCheck()) { env->ExceptionClear(); }
		env->DeleteLocalRef(jstr_permission);

		// permission_info may be null if the permission string we decided on
		// isn't known by the device.
		if (permission_info == nullptr) {
			local.present[p] = permission_state_unavailable;
			continue;
		}

		jclass    class_permissionInfo         = env->GetObjectClass(permission_info);
		jmethodID permissionInfo_getProtection = env->GetMethodID   (class_permissionInfo, "getProtection", "()I");

		jint protection_level_core;
		if (permissionInfo_getProtection) {
			// getProtection may only be available in newer Android versions
			protection_level_core = env->CallIntMethod(permission_info, permissionInfo_getProtection);
		} else {
			// protectionLevel is deprecated
			jfieldID permissionInfo_protectionLevel = env->GetFieldID (class_permissionInfo, "protectionLevel", "I");
			jint     protection_level               = env->GetIntField(permission_info, permissionInfo_protectionLevel);
			protection_level_core = (protection_level & PROTECTION_MASK_BASE);
		}
		
		local.requires_interaction[p] = (is_interactive == JNI_TRUE) || (protection_level_core == PERMISSION_PROTECTION_DANGEROUS);
		local.present             [p] = permission_state_capable;
		log_infof("Found manifest permission for %s", local.permission_str[p]);

		env->DeleteLocalRef(permission_info);
		env->DeleteLocalRef(class_permissionInfo);
	}

	env->DeleteLocalRef(class_string);
	env->DeleteLocalRef(class_packageInfo);
	env->DeleteLocalRef(class_packageManager);
	env->DeleteLocalRef(jobj_packageManager);
	env->DeleteLocalRef(jobj_packageName);
	env->DeleteLocalRef(jobj_packageInfo);
	if (jobj_requestedPermissions) env->DeleteLocalRef(jobj_requestedPermissions);
}

///////////////////////////////////////////

void _permission_request_permission(const char* permission) {
	JNIEnv* env      = (JNIEnv*)backend_android_get_jni_env ();
	jobject activity = (jobject)backend_android_get_activity();

	jclass       class_string         = env->FindClass("java/lang/String");
	jstring      jobj_permission      = env->NewStringUTF  (permission);
	jobjectArray jobj_permission_list = env->NewObjectArray(1, class_string, NULL);

	static const jint SK_REQUEST_CODE_PERMS = 0x534B;

	env->SetObjectArrayElement(jobj_permission_list, 0, jobj_permission);
	env->CallVoidMethod       (activity, local.activity_requestPermissions, jobj_permission_list, SK_REQUEST_CODE_PERMS);

	env->DeleteLocalRef(jobj_permission);
	env->DeleteLocalRef(jobj_permission_list);
	env->DeleteLocalRef(class_string);
}

///////////////////////////////////////////

#else

///////////////////////////////////////////

bool permission_init() {
	return true;
}

///////////////////////////////////////////

void permission_shutdown() {
}

///////////////////////////////////////////

permission_state_ permission_state(permission_type_ permission) {
	return permission_state_granted;
}

///////////////////////////////////////////

bool32_t permission_is_interactive(permission_type_ permission) {
	return false;
}

///////////////////////////////////////////

void permission_request(permission_type_ permission) {
}

#endif

}