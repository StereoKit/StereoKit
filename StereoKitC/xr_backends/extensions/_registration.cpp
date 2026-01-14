/* SPDX-License-Identifier: MIT */
/* The authors below grant copyright rights under the MIT license:
 * Copyright (c) 2025 Nick Klingensmith
 * Copyright (c) 2025 Qualcomm Technologies, Inc.
 */

#include "../openxr.h"
#include "../../asset_types/anchor.h"

#include "ext_management.h"
#include "_registration.h"

#include "composition_depth.h"
#include "debug_utils.h"
#include "ext_interaction_render_model.h"
#include "ext_render_model.h"
#include "eye_interaction.h"
#include "fb_colorspace.h"
#include "vulkan_enable.h"
#include "hand_tracking.h"
#include "hand_mesh.h"
#include "input_profiles.h"
#include "loader_init.h"
#include "palm_pose.h"
#include "android_create_instance.h"
#include "android_thread.h"
#include "overlay.h"
#include "time.h"
#include "oculus_audio.h"
#include "msft_anchors.h"
#include "msft_bridge.h"
#include "msft_observer.h"
#include "msft_anchor_interop.h"
#include "msft_scene_understanding.h"

///////////////////////////////////////////

namespace sk {

///////////////////////////////////////////

bool ext_registration() {
	// These extensions require deep integration, so we just request them here.
	if (ext_management_get_use_min() == false) {
		ext_management_request_ext(XR_EXT_LOCAL_FLOOR_EXTENSION_NAME);
		ext_management_request_ext(XR_MSFT_UNBOUNDED_REFERENCE_SPACE_EXTENSION_NAME);
	}

	xr_ext_debug_utils_register();
	xr_ext_time_register();
	xr_ext_android_create_instance_register();
	xr_ext_android_thread_register();
	xr_ext_vulkan_enable_register();
	xr_ext_hand_tracking_register();
	xr_ext_msft_hand_mesh_register();
	xr_ext_msft_scene_understanding_register();
	xr_ext_msft_spatial_anchors_register();
	xr_ext_fb_colorspace_register();
	xr_ext_msft_observer_register();
	xr_ext_composition_depth_register();
	xr_ext_overlay_register();
	xr_ext_oculus_audio_register();
	xr_ext_msft_bridge_register();
	xr_ext_msft_anchor_interop_register();
	xr_ext_interaction_render_model_register();
	xr_ext_render_model_register();

	// Input extensions all must go before the oxri/input system
	xr_ext_palm_pose_register                    ();
	xr_profile_ext_hp_mr_controller_register     ();
	xr_profile_ext_hand_interaction_register     ();
	xr_profile_msft_hand_interaction_register    ();
	xr_profile_bd_controller_interaction_register();
	xr_profile_ext_eye_gaze_register             ();
	oxri_register();

	anchors_register();

	return true;
}

///////////////////////////////////////////

} // namespace sk