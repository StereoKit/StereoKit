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
	permission_state_ present             [permission_max];
	bool              requires_interaction[permission_max];
	const char*       permission_str      [permission_max];
	xr_runtime_       runtime;

	jclass    class_activity;
	jmethodID activity_checkSelfPermission;
};
static permission_state_t local;

const int32_t PERMISSION_DENIED = -1;
const int32_t PERMISSION_GRANTED = 0;
const int32_t PERMISSION_PROTECTION_DANGEROUS = 0x1;

///////////////////////////////////////////

bool        android_check_app_permission      (const char* permission);
void        android_check_manifest_permissions(void);
void        android_request_permission        (const char* permission);
xr_runtime_ _permissions_check_runtime        (void);

///////////////////////////////////////////

bool permissions_init() {
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


	// Check what permissions we've got in the manifest
	local.runtime = _permissions_check_runtime();
	android_check_manifest_permissions();

	return true;
}

///////////////////////////////////////////

void permissions_shutdown() {
	JNIEnv* env = (JNIEnv*)backend_android_get_jni_env();
	env->DeleteGlobalRef(local.class_activity);

	local = {};
}

///////////////////////////////////////////

xr_runtime_ _permissions_check_runtime() {
	const char* runtime_name = device_get_runtime();
	xr_runtime_ result       = xr_runtime_unknown;
	if      (string_startswith(runtime_name, "Meta"  )) result = xr_runtime_meta;
	else if (string_startswith(runtime_name, "Oculus")) result = xr_runtime_meta;
	else if (string_startswith(runtime_name, "Monado")) result = xr_runtime_monado;
	else if (string_startswith(runtime_name, "Moohan")) result = xr_runtime_android_xr;
	else if (string_startswith(runtime_name, "Vive"  )) result = xr_runtime_vive;

	return result;
}

///////////////////////////////////////////

bool32_t permissions_is_interactive(permission_ permission) {
	return local.requires_interaction[permission];
}

///////////////////////////////////////////

permission_state_ permissions_state(permission_ permission) {
	if (local.present[permission] == permission_state_capable) {
		if (android_check_app_permission(local.permission_str[permission]))
			local.present[permission] = permission_state_granted;
	}
	return local.present[permission];
}

///////////////////////////////////////////

void permissions_request(permission_ permission) {
	if (local.permission_str[permission] == NULL)
		return;

	log_infof("Requesting permission for %s", local.permission_str[permission]);

	android_request_permission(local.permission_str[permission]);
}

///////////////////////////////////////////

bool android_check_app_permission(const char* permission) {
	JNIEnv* env      = (JNIEnv*)backend_android_get_jni_env();
	jobject activity = (jobject)backend_android_get_activity();

	jstring jobj_permission = env->NewStringUTF(permission);
	jint    result          = env->CallIntMethod(activity, local.activity_checkSelfPermission, jobj_permission);
	env->DeleteLocalRef(jobj_permission);

	return result == PERMISSION_GRANTED;
}

///////////////////////////////////////////

bool _jni_manifest_has(JNIEnv* env, jobjectArray permission_list, jsize list_count, jmethodID string_equals, const char* permission_str) {
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

void android_check_manifest_permissions() {
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
	jsize       length  = env->GetArrayLength(jobj_requestedPermissions);
	xr_runtime_ runtime = _permissions_check_runtime();
	for (int32_t p = 0; p < permission_max; p++) {
		#define CHECK_PERMISSION(s) if (!local.permission_str[p] && _jni_manifest_has(env, jobj_requestedPermissions, length, string_equals, s)) {local.permission_str[p] = s;}
		switch ((permission_)p) {

		case permission_microphone:
			CHECK_PERMISSION("android.permission.RECORD_AUDIO"); break;

		case permission_eye_tracking: { switch (runtime) {
			case xr_runtime_android_xr:
				CHECK_PERMISSION("android.permission.EYE_TRACKING_FINE"); break;
			case xr_runtime_meta:
				CHECK_PERMISSION("com.oculus.permission.EYE_TRACKING");
				CHECK_PERMISSION("horizonos.permission.EYE_TRACKING" ); break;
			default: break;
		} } break;

		case permission_hand_tracking: { switch (runtime) {
			case xr_runtime_android_xr:
				CHECK_PERMISSION("android.permission.HAND_TRACKING"); break;
			case xr_runtime_meta:
				CHECK_PERMISSION("com.oculus.permission.HAND_TRACKING");
				CHECK_PERMISSION("horizonos.permission.HAND_TRACKING" ); break;
			default: break;
		} } break;

		case permission_scene: { switch (runtime) {
			case xr_runtime_meta:
				CHECK_PERMISSION("com.oculus.permission.USE_SCENE");
				CHECK_PERMISSION("horizonos.permission.USE_SCENE" ); break;
			default: break;
		} } break;
		default: break;
		}
		#undef CHECK_PERMISSION

		if (local.permission_str[p] != nullptr) {
			jstring  jstr_permission = env->NewStringUTF(local.permission_str[p]);
			jboolean is_interactive  = env->CallBooleanMethod(activity, activity_shouldShowRationale, jstr_permission);
			jobject  permission_info = env->CallObjectMethod(jobj_packageManager, packageManager_getPermissionInfo, jstr_permission, 0);
			env->DeleteLocalRef(jstr_permission);

			jclass   class_permissionInfo           = env->GetObjectClass(permission_info);
			jfieldID permissionInfo_protectionLevel = env->GetFieldID(class_permissionInfo, "protectionLevel", "I");
			jint     protection_level               = env->GetIntField(permission_info, permissionInfo_protectionLevel);
			env->DeleteLocalRef(class_permissionInfo);

			local.requires_interaction[p] = is_interactive || ((protection_level & PERMISSION_PROTECTION_DANGEROUS) != 0) ? true : false;
			local.present             [p] = permission_state_capable;
			log_infof("Found manifest permission for %s", local.permission_str[p]);
		}
	}

	env->DeleteLocalRef(class_string);
	env->DeleteLocalRef(class_packageManager);
	env->DeleteLocalRef(jobj_packageManager);
	env->DeleteLocalRef(jobj_packageName);
	env->DeleteLocalRef(jobj_packageInfo);
	env->DeleteLocalRef(jobj_requestedPermissions);
}

///////////////////////////////////////////

void android_request_permission(const char* permission) {
	JNIEnv* env      = (JNIEnv*)backend_android_get_jni_env ();
	jobject activity = (jobject)backend_android_get_activity();

	jmethodID activity_requestPermissions = env->GetMethodID(local.class_activity, "requestPermissions",  "([Ljava/lang/String;I)V");

	jstring      jobj_permission      = env->NewStringUTF  (permission);
	jobjectArray jobj_permission_list = env->NewObjectArray(1, env->FindClass("java/lang/String"), NULL);

	env->SetObjectArrayElement(jobj_permission_list, 0, jobj_permission);
	env->CallVoidMethod       (activity, activity_requestPermissions, jobj_permission_list, 0);

	env->DeleteLocalRef(jobj_permission);
	env->DeleteLocalRef(jobj_permission_list);
}

///////////////////////////////////////////

#else

///////////////////////////////////////////

bool permissions_init() {
	return true;
}

///////////////////////////////////////////

void permissions_shutdown() {
}

///////////////////////////////////////////

permission_state_ permissions_state(permission_ permission) {
	return permission_state_granted;
}

///////////////////////////////////////////

bool32_t permissions_is_interactive(permission_ permission) {
	return false;
}

///////////////////////////////////////////

void permissions_request(permission_ permission) {
}

#endif

}