/* SPDX-License-Identifier: MIT */
/* The authors below grant copyright rights under the MIT license:
 * Copyright (c) 2026 Nick Klingensmith
 */

#include "meta_environment_depth.h"

#include "../../asset_types/texture.h"
#include "../../asset_types/texture_.h"
#include "../../log.h"
#include "../../sk_math.h"
#include "../../sk_memory.h"
#include "../../systems/sensor.h"
#include "../openxr_platform.h"
#include "ext_management.h"

#include <stdint.h>

#define XR_META_ENVIRONMENT_DEPTH_FUNCTIONS(X)             \
	X(xrCreateEnvironmentDepthProviderMETA)                \
	X(xrDestroyEnvironmentDepthProviderMETA)               \
	X(xrStartEnvironmentDepthProviderMETA)                 \
	X(xrStopEnvironmentDepthProviderMETA)                  \
	X(xrAcquireEnvironmentDepthImageMETA)                  \
	X(xrCreateEnvironmentDepthSwapchainMETA)               \
	X(xrDestroyEnvironmentDepthSwapchainMETA)              \
	X(xrEnumerateEnvironmentDepthSwapchainImagesMETA)      \
	X(xrGetEnvironmentDepthSwapchainStateMETA)             \
	X(xrSetEnvironmentDepthHandRemovalMETA)
OPENXR_DEFINE_FN_STATIC(XR_META_ENVIRONMENT_DEPTH_FUNCTIONS);

namespace sk {

///////////////////////////////////////////

typedef struct xr_environment_depth_state_t {
	bool                                 available;
	bool                                 running;
	bool                                 supports_hand_removal;
	sensor_depth_caps_                   active_flags;
	XrEnvironmentDepthProviderMETA       provider;
	XrEnvironmentDepthSwapchainMETA      swapchain;
	XrEnvironmentDepthSwapchainStateMETA swapchain_state;
	uint32_t                             image_count;
	XrSwapchainImageVulkanKHR*           images;
	tex_t                                depth_tex;
	bool                                 depth_tex_initialized;
	sensor_depth_frame_t                 latest_frame;
	bool                                 has_latest_frame;
	XrTime                               last_stop_time;
} xr_environment_depth_state_t;
static xr_environment_depth_state_t local = {};

///////////////////////////////////////////

xr_system_ xr_ext_meta_environment_depth_initialize(void*);
void       xr_ext_meta_environment_depth_shutdown  (void*);
void       xr_ext_meta_environment_depth_destroy   ();

///////////////////////////////////////////

inline pose_t xr_to_pose(const XrPosef& pose) {
	pose_t result = {};
	result.position    = { pose.position.x,    pose.position.y,    pose.position.z    };
	result.orientation = { pose.orientation.x, pose.orientation.y, pose.orientation.z, pose.orientation.w };
	return result;
}

///////////////////////////////////////////

inline fov_info_t xr_to_fov(const XrFovf& fov) {
	fov_info_t result = {};
	result.left   = fov.angleLeft  * rad2deg;
	result.right  = fov.angleRight * rad2deg;
	result.top    = fov.angleUp    * rad2deg;
	result.bottom = fov.angleDown  * rad2deg;
	return result;
}

///////////////////////////////////////////

void xr_ext_meta_environment_depth_register() {
	local = {};

	xr_system_t sys = {};
	sys.request_exts[sys.request_ext_count++] = XR_META_ENVIRONMENT_DEPTH_EXTENSION_NAME;
	sys.evt_initialize = { xr_ext_meta_environment_depth_initialize };
	sys.evt_shutdown   = { xr_ext_meta_environment_depth_shutdown };
	ext_management_sys_register(sys);
}

///////////////////////////////////////////

xr_system_ xr_ext_meta_environment_depth_initialize(void*) {
	if (!backend_openxr_ext_enabled(XR_META_ENVIRONMENT_DEPTH_EXTENSION_NAME))
		return xr_system_fail;

	OPENXR_LOAD_FN_RETURN(XR_META_ENVIRONMENT_DEPTH_FUNCTIONS, xr_system_fail);

	XrSystemEnvironmentDepthPropertiesMETA properties_depth = { XR_TYPE_SYSTEM_ENVIRONMENT_DEPTH_PROPERTIES_META };
	XrSystemProperties                     properties       = { XR_TYPE_SYSTEM_PROPERTIES };
	properties.next = &properties_depth;

	XrResult result = xrGetSystemProperties(xr_instance, xr_system_id, &properties);
	if (XR_FAILED(result)) {
		log_warnf("XR_META_environment_depth: xrGetSystemProperties failed: [%s]", openxr_string(result));
		return xr_system_fail;
	}
	if (properties_depth.supportsEnvironmentDepth == XR_FALSE)
		return xr_system_fail;

	local.supports_hand_removal = properties_depth.supportsHandRemoval == XR_TRUE;

	local.available = true;
	return xr_system_succeed;
}

///////////////////////////////////////////

void xr_ext_meta_environment_depth_destroy() {
	if (local.running && local.provider != XR_NULL_HANDLE && xrStopEnvironmentDepthProviderMETA != nullptr) {
		xrStopEnvironmentDepthProviderMETA(local.provider);
	}
	local.running      = false;
	local.active_flags = sensor_depth_caps_none;

	local.latest_frame     = {};
	local.has_latest_frame = false;

	if (local.depth_tex != nullptr) {
		tex_release(local.depth_tex);
		local.depth_tex = nullptr;
	}
	local.depth_tex_initialized = false;

	if (local.images != nullptr) {
		sk_free(local.images);
		local.images = nullptr;
	}
	local.image_count = 0;

	if (local.swapchain != XR_NULL_HANDLE && xrDestroyEnvironmentDepthSwapchainMETA != nullptr) {
		xrDestroyEnvironmentDepthSwapchainMETA(local.swapchain);
		local.swapchain = XR_NULL_HANDLE;
	}
	if (local.provider != XR_NULL_HANDLE && xrDestroyEnvironmentDepthProviderMETA != nullptr) {
		xrDestroyEnvironmentDepthProviderMETA(local.provider);
		local.provider = XR_NULL_HANDLE;
	}
}

///////////////////////////////////////////

void xr_ext_meta_environment_depth_shutdown(void*) {
	xr_ext_meta_environment_depth_destroy();
	OPENXR_CLEAR_FN(XR_META_ENVIRONMENT_DEPTH_FUNCTIONS);
	local = {};
}

///////////////////////////////////////////

bool xr_ext_meta_environment_depth_available() {
	return local.available;
}

///////////////////////////////////////////

bool xr_ext_meta_environment_depth_running() {
	return local.running;
}

///////////////////////////////////////////

sensor_depth_caps_ xr_ext_meta_environment_depth_get_capabilities() {
	sensor_depth_caps_ caps = sensor_depth_caps_none;
	if (local.supports_hand_removal)
		caps |= sensor_depth_caps_hand_removal;
	return caps;
}

///////////////////////////////////////////

bool xr_ext_meta_environment_depth_start(sensor_depth_caps_ flags) {
	if (!local.available || xrStartEnvironmentDepthProviderMETA == nullptr)
		return false;
	if (local.running)
		return true;

	if (local.provider == XR_NULL_HANDLE) {
		XrEnvironmentDepthProviderCreateInfoMETA provider_info = { XR_TYPE_ENVIRONMENT_DEPTH_PROVIDER_CREATE_INFO_META };
		XrResult result = xrCreateEnvironmentDepthProviderMETA(xr_session, &provider_info, &local.provider);
		if (XR_FAILED(result)) {
			log_warnf("XR_META_environment_depth: xrCreateEnvironmentDepthProviderMETA failed: [%s]", openxr_string(result));
			xr_ext_meta_environment_depth_destroy();
			return false;
		}
	}

	if (local.swapchain == XR_NULL_HANDLE) {
		XrEnvironmentDepthSwapchainCreateInfoMETA swapchain_info = { XR_TYPE_ENVIRONMENT_DEPTH_SWAPCHAIN_CREATE_INFO_META };
		XrResult result = xrCreateEnvironmentDepthSwapchainMETA(local.provider, &swapchain_info, &local.swapchain);
		if (XR_FAILED(result)) {
			log_warnf("XR_META_environment_depth: xrCreateEnvironmentDepthSwapchainMETA failed: [%s]", openxr_string(result));
			xr_ext_meta_environment_depth_destroy();
			return false;
		}

		local.swapchain_state = { XR_TYPE_ENVIRONMENT_DEPTH_SWAPCHAIN_STATE_META };
		result = xrGetEnvironmentDepthSwapchainStateMETA(local.swapchain, &local.swapchain_state);
		if (XR_FAILED(result)) {
			log_warnf("XR_META_environment_depth: xrGetEnvironmentDepthSwapchainStateMETA failed: [%s]", openxr_string(result));
			xr_ext_meta_environment_depth_destroy();
			return false;
		}

		result = xrEnumerateEnvironmentDepthSwapchainImagesMETA(local.swapchain, 0, &local.image_count, nullptr);
		if (XR_FAILED(result) || local.image_count == 0) {
			log_warnf("XR_META_environment_depth: xrEnumerateEnvironmentDepthSwapchainImagesMETA(count) failed: [%s]", openxr_string(result));
			xr_ext_meta_environment_depth_destroy();
			return false;
		}

		local.images = sk_malloc_t(XrSwapchainImageVulkanKHR, local.image_count);
		if (local.images == nullptr) {
			log_warn("XR_META_environment_depth: Failed to allocate swapchain image array.");
			xr_ext_meta_environment_depth_destroy();
			return false;
		}
		for (uint32_t i = 0; i < local.image_count; i++) {
			local.images[i] = { XR_TYPE_SWAPCHAIN_IMAGE_VULKAN_KHR };
		}

		result = xrEnumerateEnvironmentDepthSwapchainImagesMETA(
			local.swapchain,
			local.image_count,
			&local.image_count,
			(XrSwapchainImageBaseHeader*)local.images);
		if (XR_FAILED(result)) {
			log_warnf("XR_META_environment_depth: xrEnumerateEnvironmentDepthSwapchainImagesMETA(images) failed: [%s]", openxr_string(result));
			xr_ext_meta_environment_depth_destroy();
			return false;
		}

		// Per the XR_META_environment_depth spec, the Vulkan swapchain format is
		// VK_FORMAT_D16_UNORM and it is a 2D array texture with 2 layers
		// (layer 0 = left eye, layer 1 = right eye)
		log_infof("XR_META_environment_depth: swapchain images=%u size=%dx%d fmt=D16_UNORM layers=2",
			local.image_count, local.swapchain_state.width, local.swapchain_state.height);
	}

	XrResult result = xrStartEnvironmentDepthProviderMETA(local.provider);
	if (XR_FAILED(result)) {
		log_warnf("XR_META_environment_depth: xrStartEnvironmentDepthProviderMETA failed: [%s]", openxr_string(result));
		return false;
	}

	local.running      = true;
	local.active_flags = sensor_depth_caps_none;

	// Create the depth texture eagerly so callers can query format/dimensions
	// immediately. The VkImage surface binding happens on the first frame.
	if (local.depth_tex == nullptr) {
		local.depth_tex = tex_create(tex_type_image_nomips, tex_format_depth16);
		tex_set_id     (local.depth_tex, "default/tex_sensor_depth");
		tex_set_sample (local.depth_tex, tex_sample_point);
		tex_set_address(local.depth_tex, tex_address_clamp);
		local.depth_tex_initialized = false;
	}

	// Apply initial flags
	xr_ext_meta_environment_depth_set_caps(flags);
	return true;
}

///////////////////////////////////////////

void xr_ext_meta_environment_depth_stop() {
	if (!local.running || local.provider == XR_NULL_HANDLE || xrStopEnvironmentDepthProviderMETA == nullptr)
		return;

	// Remember the last frame time so we can reject stale frames after restart
	if (local.has_latest_frame) {
		XrTime frame_time    = local.latest_frame.capture_time != 0 ? local.latest_frame.capture_time : local.latest_frame.display_time;
		local.last_stop_time = frame_time;
	}

	XrResult result = xrStopEnvironmentDepthProviderMETA(local.provider);
	if (XR_FAILED(result)) {
		log_warnf("XR_META_environment_depth: xrStopEnvironmentDepthProviderMETA failed: [%s]", openxr_string(result));
	}
	local.running      = false;
	local.active_flags = sensor_depth_caps_none;

	local.latest_frame     = {};
	local.has_latest_frame = false;
}

///////////////////////////////////////////

bool xr_ext_meta_environment_depth_set_caps(sensor_depth_caps_ flags) {
	if (!local.running) return false;

	// Hand removal: apply if flag changed
	bool want_hand_removal = (flags & sensor_depth_caps_hand_removal) != 0;
	bool have_hand_removal = (local.active_flags & sensor_depth_caps_hand_removal) != 0;
	if (want_hand_removal != have_hand_removal && local.supports_hand_removal && xrSetEnvironmentDepthHandRemovalMETA != nullptr) {
		XrEnvironmentDepthHandRemovalSetInfoMETA info = { XR_TYPE_ENVIRONMENT_DEPTH_HAND_REMOVAL_SET_INFO_META };
		info.enabled = want_hand_removal ? XR_TRUE : XR_FALSE;
		XrResult result = xrSetEnvironmentDepthHandRemovalMETA(local.provider, &info);
		if (XR_FAILED(result)) {
			log_warnf("XR_META_environment_depth: xrSetEnvironmentDepthHandRemovalMETA failed: [%s]", openxr_string(result));
		}
	}

	local.active_flags = flags;
	return true;
}

///////////////////////////////////////////

tex_t xr_ext_meta_environment_depth_get_texture() {
	return local.depth_tex;
}

///////////////////////////////////////////

bool xr_ext_meta_environment_depth_try_get_latest(sensor_depth_frame_t* out_frame) {
	if (out_frame == nullptr || !local.available)
		return false;

	if (!local.has_latest_frame)
		return false;

	*out_frame = local.latest_frame;
	return true;
}

///////////////////////////////////////////

void xr_ext_meta_environment_depth_update_frame(XrTime display_time) {
	if (!local.running || !local.available || local.provider == XR_NULL_HANDLE || xrAcquireEnvironmentDepthImageMETA == nullptr || xr_app_space == XR_NULL_HANDLE)
		return;

	XrEnvironmentDepthImageAcquireInfoMETA acquire_info = { XR_TYPE_ENVIRONMENT_DEPTH_IMAGE_ACQUIRE_INFO_META };
	acquire_info.space       = xr_app_space;
	acquire_info.displayTime = display_time;

	// Chain the v2 timestamp struct. If the runtime is v1, it will be ignored.
	// Initialize captureTime to 0 per spec so we can detect if it was populated.
	XrEnvironmentDepthImageTimestampMETA timestamp_info = { XR_TYPE_ENVIRONMENT_DEPTH_IMAGE_TIMESTAMP_META };
	timestamp_info.captureTime = 0;

	XrEnvironmentDepthImageMETA image_info = { XR_TYPE_ENVIRONMENT_DEPTH_IMAGE_META };
	image_info.next     = &timestamp_info;
	image_info.views[0] = { XR_TYPE_ENVIRONMENT_DEPTH_IMAGE_VIEW_META };
	image_info.views[1] = { XR_TYPE_ENVIRONMENT_DEPTH_IMAGE_VIEW_META };

	XrResult result = xrAcquireEnvironmentDepthImageMETA(local.provider, &acquire_info, &image_info);
	if (result == XR_ENVIRONMENT_DEPTH_NOT_AVAILABLE_META)
		return;
	if (XR_FAILED(result)) {
		log_warnf("XR_META_environment_depth: xrAcquireEnvironmentDepthImageMETA failed: [%s]", openxr_string(result));
		return;
	}

	if (local.last_stop_time != 0) {
		XrTime frame_time = timestamp_info.captureTime != 0 ? timestamp_info.captureTime : display_time;
		if (frame_time <= local.last_stop_time) {
			return;
		}
		local.last_stop_time = 0;
	}

	if (image_info.swapchainIndex >= local.image_count || local.images == nullptr) {
		log_warnf("XR_META_environment_depth: invalid swapchain index %u (image_count=%u, images=%p)", image_info.swapchainIndex, local.image_count, (void*)local.images);
		return;
	}

	const int32_t width       = (int32_t)local.swapchain_state.width;
	const int32_t height      = (int32_t)local.swapchain_state.height;
	const int32_t array_count = 2;
	const int64_t native_fmt  = tex_fmt_to_native(tex_format_depth16);
	void*         image_ptr   = (void*)(uintptr_t)local.images[image_info.swapchainIndex].image;

	if (!local.depth_tex_initialized) {
		tex_set_surface(local.depth_tex, image_ptr, tex_type_image_nomips, native_fmt, width, height, array_count, 1, false);
		local.depth_tex_initialized = true;
	} else {
		skr_tex_external_update_t update = {};
		update.image          = (VkImage)image_ptr;
		update.view           = VK_NULL_HANDLE;
		update.current_layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		skr_tex_update_external(&local.depth_tex->gpu_tex, update);
	}

	// Build current frame metadata (GPU texture always matches these)
	sensor_depth_frame_t frame = {};
	frame.display_time      = display_time;
	frame.capture_time      = timestamp_info.captureTime;
	frame.width             = local.swapchain_state.width;
	frame.height            = local.swapchain_state.height;
	frame.near_z            = image_info.nearZ;
	frame.far_z             = image_info.farZ;
	frame.views[0].pose     = xr_to_pose(image_info.views[0].pose);
	frame.views[0].fov      = xr_to_fov (image_info.views[0].fov );
	frame.views[1].pose     = xr_to_pose(image_info.views[1].pose);
	frame.views[1].fov      = xr_to_fov (image_info.views[1].fov );

	local.latest_frame     = frame;
	local.has_latest_frame = true;

	// Use a unique frame identifier to avoid redundant readbacks
	uint64_t frame_id = (uint64_t)(timestamp_info.captureTime != 0 ? timestamp_info.captureTime : display_time);

	sensor_readback_update(local.depth_tex, (int32_t)frame.width, (int32_t)frame.height, array_count, &frame, sizeof(frame), frame_id);
}

///////////////////////////////////////////

} // namespace sk