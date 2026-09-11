// SPDX-License-Identifier: MIT
// The authors below grant copyright rights under the MIT license:
// Copyright (c) 2025 Nick Klingensmith
// Copyright (c) 2025 Qualcomm Technologies, Inc.

#include "permission.h"
#include "../sk_memory.h"
#include "../xr_backends/openxr.h"

namespace sk {

///////////////////////////////////////////

#if defined(SK_OS_ANDROID)

///////////////////////////////////////////

#include <sk_app.h>

// StereoKit names permissions after the feature they unlock; sk_app owns the
// Android permission state, keyed on the system permission string. Features
// that share a string therefore share a state for free.
struct permission_state_t {
	const char* permission_str[permission_type_max]; // null when unknown here
};
static permission_state_t local;

///////////////////////////////////////////

const char* _permission_check_string(permission_type_ type, xr_runtime_ runtime) {
	const char* result = nullptr;

	// 'undeclared' is sk_app for 'not in the AndroidManifest', which is how a
	// runtime with several official strings picks the one the app shipped.
	#define PERMISSION_CHECK(r, s) if (runtime == r && !result && ska_android_permission_get(s) != ska_android_permission_undeclared) { result = s; }
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
	case permission_type_ambient_estimation:
		PERMISSION_SET  (xr_runtime_android_xr, "android.permission.SCENE_UNDERSTANDING_COARSE");
		PERMISSION_CHECK(xr_runtime_meta,       "com.oculus.permission.USE_SCENE");
		PERMISSION_SET  (xr_runtime_meta,       "horizonos.permission.USE_SCENE");
		PERMISSION_SET  (xr_runtime_pico,       "com.picovr.permission.SPATIAL_DATA");
		break;
	// Reflection estimates and depth sensing expose detailed data, which
	// Android XR splits out as a 'fine' permission. Other runtimes cover
	// them with their single scene permission, same as ambient above.
	case permission_type_reflection_estimation:
	case permission_type_depth_sensing:
		PERMISSION_SET  (xr_runtime_android_xr, "android.permission.SCENE_UNDERSTANDING_FINE");
		PERMISSION_CHECK(xr_runtime_meta,       "com.oculus.permission.USE_SCENE");
		PERMISSION_SET  (xr_runtime_meta,       "horizonos.permission.USE_SCENE");
		PERMISSION_SET  (xr_runtime_pico,       "com.picovr.permission.SPATIAL_DATA");
		break;
	case permission_type_anchors:
		PERMISSION_SET  (xr_runtime_android_xr, "android.permission.SCENE_UNDERSTANDING_COARSE");
		PERMISSION_CHECK(xr_runtime_meta,       "com.oculus.permission.USE_ANCHOR_API");
		PERMISSION_SET  (xr_runtime_meta,       "horizonos.permission.USE_ANCHOR_API");
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

bool permission_init() {
	local = {};

	xr_runtime_ runtime = openxr_get_known_runtime();
	for (int32_t i = 0; i < permission_type_max; i++)
		local.permission_str[i] = _permission_check_string((permission_type_)i, runtime);

	return true;
}

///////////////////////////////////////////

void permission_shutdown() {
	local = {};
}

///////////////////////////////////////////

static permission_state_ _permission_from_ska(ska_android_permission_ state) {
	switch (state) {
	case ska_android_permission_granted: return permission_state_granted;
	case ska_android_permission_askable: return permission_state_capable;
	case ska_android_permission_pending: return permission_state_requesting;
	case ska_android_permission_denied:  return permission_state_denied;
	case ska_android_permission_blocked: return permission_state_blocked;
	default:                             return permission_state_unavailable;
	}
}

///////////////////////////////////////////

bool32_t permission_is_interactive(permission_type_ permission) {
	const char* str = local.permission_str[permission];
	return str != nullptr && ska_android_permission_prompts(str);
}

///////////////////////////////////////////

permission_state_ permission_state(permission_type_ permission) {
	const char* str = local.permission_str[permission];
	if (str == nullptr) return permission_state_unknown;
	return _permission_from_ska(ska_android_permission_get(str));
}

///////////////////////////////////////////

void permission_request(const permission_type_* in_arr_permissions, int32_t permission_count) {
	if (permission_count <= 0) return;

	const char** permission_strs = sk_malloc_t(const char*, permission_count);
	int32_t      valid_count     = 0;
	for (int32_t i = 0; i < permission_count; i++) {
		permission_type_ p = in_arr_permissions[i];
		if (p < 0 || p >= permission_type_max || local.permission_str[p] == nullptr) {
			log_warnf("Permission string for 0x%X unknown on current platform", p);
			continue;
		}
		permission_strs[valid_count++] = local.permission_str[p];
	}

	// sk_app collapses duplicate and already-settled names into one prompt,
	// so features sharing a system permission need no special handling here.
	if (valid_count > 0 && !ska_android_permission_request(permission_strs, valid_count))
		log_warnf("Permission request failed: %s", ska_error_get());

	sk_free(permission_strs);
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

void permission_request(const permission_type_* in_arr_permissions, int32_t permission_count) {
}

#endif

}