/* SPDX-License-Identifier: MIT */
/* The authors below grant copyright rights under the MIT license:
 * Copyright (c) 2019-2024 Nick Klingensmith
 * Copyright (c) 2024 Qualcomm Technologies, Inc.
 */

#pragma once

#include "../platforms/platform.h"
#if defined(SK_XR_OPENXR)

#if defined(SK_OS_ANDROID)
	#define XR_USE_PLATFORM_ANDROID
	#define XR_USE_TIMESPEC
	#define XR_TIME_EXTENSION XR_KHR_CONVERT_TIMESPEC_TIME_EXTENSION_NAME
	#define XR_USE_GRAPHICS_API_OPENGL_ES

#elif defined(SK_OS_LINUX)
	#define XR_USE_PLATFORM_EGL
	#define XR_USE_GRAPHICS_API_OPENGL_ES

	#define XR_USE_TIMESPEC
	#define XR_TIME_EXTENSION XR_KHR_CONVERT_TIMESPEC_TIME_EXTENSION_NAME

#elif defined(SK_OS_WEB)
	#define XR_USE_TIMESPEC
	#define XR_TIME_EXTENSION XR_KHR_CONVERT_TIMESPEC_TIME_EXTENSION_NAME
	#define XR_USE_GRAPHICS_API_OPENGL

#elif defined(SK_OS_WINDOWS) || defined(SK_OS_WINDOWS_UWP)
	#if defined(SKG_FORCE_OPENGL)
		#define XR_USE_GRAPHICS_API_OPENGL
	#else
		#define XR_USE_GRAPHICS_API_D3D11
	#endif

	#define XR_USE_PLATFORM_WIN32
	#define XR_TIME_EXTENSION XR_KHR_WIN32_CONVERT_PERFORMANCE_COUNTER_TIME_EXTENSION_NAME

#endif

#include "../stereokit.h"

#include <openxr/openxr.h>

#include <stdint.h>

typedef struct XR_MAY_ALIAS XrBaseHeader {
	XrStructureType             type;
	const void* XR_MAY_ALIAS    next;
} XrBaseHeader;

#define xr_check(xResult, message) {XrResult xr_call_result = xResult; if (XR_FAILED(xr_call_result)) {log_infof("%s [%s]", message, openxr_string(xr_call_result)); return false;}}
inline void xr_insert_next(XrBaseHeader *xr_base, XrBaseHeader *xr_next) { xr_next->next = xr_base->next; xr_base->next = xr_next; }

namespace sk {

bool openxr_init          ();
void openxr_cleanup       ();
void openxr_shutdown      ();
void openxr_step_begin    ();
void openxr_step_end      ();
bool openxr_poll_events   ();
bool openxr_render_frame  ();
void openxr_poll_actions  ();

void*         openxr_get_luid         ();
bool32_t      openxr_get_space        (XrSpace space, pose_t *out_pose, XrTime time = 0);
bool32_t      openxr_get_gaze_space   (pose_t* out_pose, XrTime& out_gaze_sample_time, XrTime time = 0);
const char*   openxr_string           (XrResult result);
void          openxr_set_origin_offset(pose_t offset);
bool          openxr_get_stage_bounds (vec2* out_size, pose_t* out_pose, XrTime time);
button_state_ openxr_space_tracked    ();

extern XrSpace    xrc_space_grip[2];
extern XrSpace    xr_app_space;
extern XrSpace    xr_head_space;
extern XrSpace    xr_gaze_space;
extern XrInstance xr_instance;
extern XrSession  xr_session;
extern XrSessionState xr_session_state;
extern XrSystemId xr_system_id;
extern bool       xr_has_bounds;
extern XrTime     xr_time;
extern XrTime     xr_eyes_sample_time;
extern vec2       xr_bounds_size;
extern pose_t     xr_bounds_pose;
extern pose_t     xr_bounds_pose_local;

#define XR_PRIMARY_CONFIG XR_VIEW_CONFIGURATION_TYPE_PRIMARY_STEREO

} // namespace sk
#endif