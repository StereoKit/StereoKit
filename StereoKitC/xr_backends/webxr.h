/* SPDX-License-Identifier: MIT */
/* The authors below grant copyright rights under the MIT license:
 * Copyright (c) 2019-2024 Nick Klingensmith
 * Copyright (c) 2024 Qualcomm Technologies, Inc.
 */

#pragma once

#include "../platforms/platform.h"

#if defined(SK_XR_WEBXR)
	#define XR_USE_TIMESPEC
	#define XR_TIME_EXTENSION XR_KHR_CONVERT_TIMESPEC_TIME_EXTENSION_NAME
	#define XR_USE_GRAPHICS_API_OPENGL

#include "../stereokit.h"

#include <stdint.h>


namespace sk {

// bool webxr_init          ();
// void webxr_cleanup       ();
// void webxr_shutdown      ();
// void webxr_step_begin    ();
// void webxr_step_end      ();
// bool webxr_poll_events   ();
// bool webxr_render_frame  ();
// void webxr_poll_actions  ();

//void*         webxr_get_luid                   ();
//bool32_t      webxr_get_space                  (XrSpace space, pose_t *out_pose, XrTime time = 0);
//bool32_t      webxr_get_gaze_space             (pose_t* out_pose, XrTime& out_gaze_sample_time, XrTime time = 0);
//pose_t        webxr_from_spatial_graph         (uint8_t spatial_graph_node_id[16], bool32_t dynamic, int64_t qpc_time);
//bool32_t      webxr_try_from_spatial_graph     (uint8_t spatial_graph_node_id[16], bool32_t dynamic, int64_t qpc_time, pose_t* out_pose);
//pose_t        webxr_from_perception_anchor     (void* perception_spatial_anchor);
//bool32_t      webxr_try_from_perception_anchor (void* perception_spatial_anchor, pose_t* out_pose);
//const char*   webxr_string                     (XrResult result);
//void          webxr_set_origin_offset          (pose_t offset);
//bool          webxr_get_stage_bounds           (vec2* out_size, pose_t* out_pose, XrTime time);
//button_state_ webxr_space_tracked              ();

//extern XrSpace    xrc_space_grip[2];
//extern XrSpace    xr_app_space;
//extern XrSpace    xr_head_space;
//extern XrSpace    xr_gaze_space;
//extern XrInstance xr_instance;
//extern XrSession  xr_session;
//extern XrSessionState xr_session_state;
//extern XrSystemId xr_system_id;
//extern bool       xr_has_articulated_hands;
//extern bool       xr_has_hand_meshes;
//extern bool       xr_has_bounds;
//extern bool       xr_has_single_pass;
//extern XrTime     xr_time;
//extern XrTime     xr_eyes_sample_time;
//extern vec2       xr_bounds_size;
//extern pose_t     xr_bounds_pose;
//extern pose_t     xr_bounds_pose_local;

#define XR_PRIMARY_CONFIG XR_VIEW_CONFIGURATION_TYPE_PRIMARY_STEREO

} // namespace sk
#endif