#pragma once

#include "../platforms/platform_utils.h"
#if defined(SK_XR_OPENXR)

#if defined(SK_OS_ANDROID)
	#define XR_USE_PLATFORM_ANDROID
	#define XR_USE_TIMESPEC
	#define XR_TIME_EXTENSION XR_KHR_CONVERT_TIMESPEC_TIME_EXTENSION_NAME
	#define XR_USE_GRAPHICS_API_OPENGL_ES

#elif defined(SK_OS_LINUX)
	#if defined(SKG_LINUX_EGL)
		#define XR_USE_PLATFORM_EGL
		#define XR_USE_GRAPHICS_API_OPENGL_ES
	#else
		#define XR_USE_PLATFORM_XLIB
		#define XR_USE_GRAPHICS_API_OPENGL
	#endif

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

#define xr_check(xResult, message) {XrResult xr_call_result = xResult; if (XR_FAILED(xr_call_result)) {log_infof(message, openxr_string(xr_call_result)); return false;}}

namespace sk {

bool openxr_init          ();
void openxr_shutdown      ();
void openxr_step_begin    ();
void openxr_step_end      ();
void openxr_poll_events   ();
bool openxr_render_frame  ();
void openxr_poll_actions  ();

void       *openxr_get_luid ();
bool32_t    openxr_get_space(XrSpace space, pose_t *out_pose, XrTime time = 0);
bool32_t    openxr_get_gaze_space(pose_t* out_pose, XrTime& out_gaze_sample_time, XrTime time = 0);
const char* openxr_string   (XrResult result);

extern XrSpace    xrc_space_grip[2];
extern XrSpace    xr_app_space;
extern XrSpace    xr_head_space;
extern XrSpace    xr_gaze_space;
extern XrInstance xr_instance;
extern XrSession  xr_session;
extern XrSessionState xr_session_state;
extern XrSystemId xr_system_id;
extern bool       xr_has_articulated_hands;
extern bool       xr_has_hand_meshes;
extern bool       xr_has_depth_lsr;
extern bool       xr_has_bounds;
extern bool       xr_has_single_pass;
extern XrTime     xr_time;
extern XrTime     xr_eyes_sample_time;
extern vec2       xr_bounds_size;
extern pose_t     xr_bounds_pose;
extern pose_t     xr_bounds_pose_local;

} // namespace sk
#endif