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

enum xr_runtime_ {
	xr_runtime_none,
	xr_runtime_unknown,
	xr_runtime_meta,
	xr_runtime_android_xr,
	xr_runtime_vive,
	xr_runtime_pico,
};

typedef struct permission_state_t {
	permission_state_ present[permission_max];
	bool              present_checked;
	xr_runtime_       runtime;
} permission_state_t;
static permission_state_t local;

void permissions_init() {
	local = {};
}

void permissions_shutdown() {
	local = {};
}

#if defined(SK_OS_ANDROID)

///////////////////////////////////////////

bool android_check_app_permission      (const char* permission);
void android_check_manifest_permissions(void);
void android_request_permission        (const char* permission);

xr_runtime_ _permissions_check_runtime() {
	if (local.runtime != xr_runtime_none)
		return local.runtime;

	const char* runtime_name = device_get_runtime();
	xr_runtime_ result = xr_runtime_unknown;
	if      (string_startswith(runtime_name, "Meta"  )) result = xr_runtime_meta;
	else if (string_startswith(runtime_name, "Oculus")) result = xr_runtime_meta;

	local.runtime = result;
	return result;
}

///////////////////////////////////////////

const char* _permissions_string(permission_ permission) {
	switch (permission) {
	case permission_microphone: return "android.permission.RECORD_AUDIO";
	case permission_eye_tracking: { switch (_permissions_check_runtime()) {
		case xr_runtime_android_xr: return "android.permission.EYE_TRACKING_FINE";
		case xr_runtime_meta:       return "com.oculus.permission.EYE_TRACKING";
		default:                    return NULL;
	} }
	case permission_hand_tracking: { switch (_permissions_check_runtime()) {
		case xr_runtime_android_xr: return "android.permission.HAND_TRACKING";
		case xr_runtime_meta:       return "com.oculus.permission.HAND_TRACKING";
		default:                    return NULL;
	} }
	case permission_scene: { switch (_permissions_check_runtime()) {
		case xr_runtime_meta: return "com.oculus.permission.USE_SCENE";
		default: return NULL;
	} }
	default: return NULL;
	}
}

///////////////////////////////////////////

bool32_t permissions_is_interactive(permission_ permission) {
	switch (permission) {
	case permission_eye_tracking: return true;
	case permission_microphone:   return true;
	case permission_hand_tracking: switch (_permissions_check_runtime()) {
		case xr_runtime_android_xr: return true;
		default:                    return false;
	}
	case permission_scene: switch (_permissions_check_runtime()) {
		case xr_runtime_meta: return true;
		default:              return true;
	}
	default: return false;
	}
}

///////////////////////////////////////////

permission_state_ permissions_state(permission_ permission) {
	if (local.present_checked == false)
		android_check_manifest_permissions();

	if (local.present[permission] == permission_state_capable) {
		const char* str = _permissions_string(permission);
		if (android_check_app_permission(str))
			local.present[permission] = permission_state_granted;
	}
	return local.present[permission];
}

///////////////////////////////////////////

void permissions_request(permission_ permission) {
	const char* str = _permissions_string(permission);
	if (str == NULL)
		return;

	log_infof("Requesting permission for %s", str);

	android_request_permission(str);
}

///////////////////////////////////////////

#include <android/native_activity.h>

const int32_t PERMISSION_DENIED  = -1;
const int32_t PERMISSION_GRANTED =  0;

///////////////////////////////////////////

bool android_check_app_permission(const char* permission) {
	JNIEnv* env      = (JNIEnv*)backend_android_get_jni_env();
	jobject activity = (jobject)backend_android_get_activity();

	jclass    class_activity               = env->GetObjectClass(activity);
	jmethodID activity_checkSelfPermission = env->GetMethodID   (class_activity, "checkSelfPermission", "(Ljava/lang/String;)I");

	jstring jobj_permission = env->NewStringUTF(permission);
	jint    result          = env->CallIntMethod(activity, activity_checkSelfPermission, jobj_permission);

	env->DeleteLocalRef(class_activity);
	env->DeleteLocalRef(jobj_permission);

	return result == PERMISSION_GRANTED;
}

///////////////////////////////////////////

void android_check_manifest_permissions() {
	JNIEnv* env      = (JNIEnv*)backend_android_get_jni_env ();
	jobject activity = (jobject)backend_android_get_activity();

	jclass       class_activity                =          env->GetObjectClass  (activity);
	jmethodID    activity_getPackageManager    =          env->GetMethodID     (class_activity, "getPackageManager", "()Landroid/content/pm/PackageManager;");
	jmethodID    activity_getPackageName       =          env->GetMethodID     (class_activity, "getPackageName", "()Ljava/lang/String;");
	jobject      jobj_packageManager           =          env->CallObjectMethod(activity, activity_getPackageManager);
	jclass       class_packageManager          =          env->GetObjectClass  (jobj_packageManager);
	jmethodID    packageManager_getPackageInfo =          env->GetMethodID     (class_packageManager, "getPackageInfo", "(Ljava/lang/String;I)Landroid/content/pm/PackageInfo;");
	jstring      jobj_packageName              = (jstring)env->CallObjectMethod(activity, activity_getPackageName);

	jint         flags = 4096; // PackageManager.GET_PERMISSIONS
	jobject      jobj_packageInfo  = env->CallObjectMethod(jobj_packageManager, packageManager_getPackageInfo, jobj_packageName, flags);
	jclass       class_packageInfo = env->GetObjectClass  (jobj_packageInfo);

	jfieldID     packageInfo_requestedPermissions =               env->GetFieldID    (class_packageInfo, "requestedPermissions", "[Ljava/lang/String;");
	jobjectArray jobj_requestedPermissions        = (jobjectArray)env->GetObjectField(jobj_packageInfo, packageInfo_requestedPermissions);

	// allocate memory for the permission strings we know about
	jstring* jobj_permissions = sk_malloc_t(jstring, permission_max);
	for (int32_t i = 0; i < permission_max; i++) {
		const char* str = _permissions_string((permission_)i);
		jobj_permissions[i] = str ? env->NewStringUTF(str) : nullptr;

		// If the string was null, mark the permission as unavailable
		if (str == nullptr)
			local.present[i] = permission_state_unavailable;
	}

	jclass       class_string    = env->GetObjectClass(jobj_permissions[0]);
	jmethodID    string_equals   = env->GetMethodID   (class_string, "equals", "(Ljava/lang/Object;)Z");

	// look through all permissions in the manifest, and match them up to ones
	// StereoKit knows about.
	jsize length = env->GetArrayLength(jobj_requestedPermissions);
	bool  result = false;
	for (jsize i = 0; i < length; i++) {
		jstring jobj_currPermission = (jstring)env->GetObjectArrayElement(jobj_requestedPermissions, i);
		for (int32_t p = 0; p < permission_max; p++) {
			if (jobj_permissions[p] == nullptr) continue;
			bool match = env->CallBooleanMethod(jobj_permissions[p], string_equals, jobj_currPermission);

			if (match) {
				local.present[p] = permission_state_capable;
				log_infof("Found manifest permission for %s", _permissions_string((permission_)p));
				break;
			}
		}
		env->DeleteLocalRef(jobj_currPermission);
	}

	for (int32_t i = 0; i < permission_max; i++) {
		env->DeleteLocalRef(jobj_permissions[i]);
	}
	sk_free(jobj_permissions);

	env->DeleteLocalRef(class_string);
	env->DeleteLocalRef(class_activity);
	env->DeleteLocalRef(class_packageManager);
	env->DeleteLocalRef(jobj_packageManager);
	env->DeleteLocalRef(jobj_packageName);
	env->DeleteLocalRef(jobj_packageInfo);
	env->DeleteLocalRef(jobj_requestedPermissions);

	local.present_checked = true;
}

///////////////////////////////////////////

void android_request_permission(const char* permission) {
	JNIEnv* env      = (JNIEnv*)backend_android_get_jni_env ();
	jobject activity = (jobject)backend_android_get_activity();

	jclass    class_activity                    = env->GetObjectClass(activity);
	jmethodID contextCompat_checkSelfPermission = env->GetMethodID   (class_activity, "checkSelfPermission", "(Ljava/lang/String;)I");
	jmethodID activity_requestPermissions       = env->GetMethodID   (class_activity, "requestPermissions",  "([Ljava/lang/String;I)V");

	jstring      jobj_permission      = env->NewStringUTF  (permission);
	jobjectArray jobj_permission_list = env->NewObjectArray(1, env->FindClass("java/lang/String"), NULL);

	env->SetObjectArrayElement(jobj_permission_list, 0, jobj_permission);
	env->CallVoidMethod       (activity, activity_requestPermissions, jobj_permission_list, 0);

	env->DeleteLocalRef(class_activity);
	env->DeleteLocalRef(jobj_permission);
	env->DeleteLocalRef(jobj_permission_list);
}

///////////////////////////////////////////

#else

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