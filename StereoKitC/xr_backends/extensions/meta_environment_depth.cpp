/* SPDX-License-Identifier: MIT */
/* The authors below grant copyright rights under the MIT license:
 * Copyright (c) 2026 Nick Klingensmith
 */

#include "meta_environment_depth.h"

#include "../../asset_types/texture.h"
#include "../../asset_types/texture_.h"
#include "../../libraries/ferr_thread.h"
#include "../../log.h"
#include "../../sk_math.h"
#include "../../sk_memory.h"
#include "../openxr_platform.h"
#include "ext_management.h"

#include <stdint.h>
#include <string.h>

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
	bool                                 hand_removal;
	XrEnvironmentDepthProviderMETA       provider;
	XrEnvironmentDepthSwapchainMETA      swapchain;
	XrEnvironmentDepthSwapchainStateMETA swapchain_state;
	uint32_t                             image_count;
	XrSwapchainImageVulkanKHR*           images;
	tex_t*                               textures;
	ft_mutex_t                           latest_mutex;
	environment_depth_frame_t            latest_frame;
	bool                                 has_latest_frame;
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
	local.latest_mutex = ft_mutex_create();
	if (local.latest_mutex == nullptr) {
		log_warn("XR_META_environment_depth: Failed to create mutex.");
		xr_ext_meta_environment_depth_destroy();
		return xr_system_fail;
	}

	XrEnvironmentDepthProviderCreateInfoMETA provider_info = { XR_TYPE_ENVIRONMENT_DEPTH_PROVIDER_CREATE_INFO_META };
	result = xrCreateEnvironmentDepthProviderMETA(xr_session, &provider_info, &local.provider);
	if (XR_FAILED(result)) {
		log_warnf("XR_META_environment_depth: xrCreateEnvironmentDepthProviderMETA failed: [%s]", openxr_string(result));
		xr_ext_meta_environment_depth_destroy();
		return xr_system_fail;
	}

	XrEnvironmentDepthSwapchainCreateInfoMETA swapchain_info = { XR_TYPE_ENVIRONMENT_DEPTH_SWAPCHAIN_CREATE_INFO_META };
	result = xrCreateEnvironmentDepthSwapchainMETA(local.provider, &swapchain_info, &local.swapchain);
	if (XR_FAILED(result)) {
		log_warnf("XR_META_environment_depth: xrCreateEnvironmentDepthSwapchainMETA failed: [%s]", openxr_string(result));
		xr_ext_meta_environment_depth_destroy();
		return xr_system_fail;
	}

	local.swapchain_state = { XR_TYPE_ENVIRONMENT_DEPTH_SWAPCHAIN_STATE_META };
	result = xrGetEnvironmentDepthSwapchainStateMETA(local.swapchain, &local.swapchain_state);
	if (XR_FAILED(result)) {
		log_warnf("XR_META_environment_depth: xrGetEnvironmentDepthSwapchainStateMETA failed: [%s]", openxr_string(result));
		xr_ext_meta_environment_depth_destroy();
		return xr_system_fail;
	}

	result = xrEnumerateEnvironmentDepthSwapchainImagesMETA(local.swapchain, 0, &local.image_count, nullptr);
	if (XR_FAILED(result) || local.image_count == 0) {
		log_warnf("XR_META_environment_depth: xrEnumerateEnvironmentDepthSwapchainImagesMETA(count) failed: [%s]", openxr_string(result));
		xr_ext_meta_environment_depth_destroy();
		return xr_system_fail;
	}

	local.images = sk_malloc_t(XrSwapchainImageVulkanKHR, local.image_count);
	if (local.images == nullptr) {
		log_warn("XR_META_environment_depth: Failed to allocate swapchain image array.");
		xr_ext_meta_environment_depth_destroy();
		return xr_system_fail;
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
		return xr_system_fail;
	}

	local.textures = sk_malloc_t(tex_t, local.image_count);
	if (local.textures == nullptr) {
		log_warn("XR_META_environment_depth: Failed to allocate texture array.");
		xr_ext_meta_environment_depth_destroy();
		return xr_system_fail;
	}
	memset(local.textures, 0, sizeof(tex_t) * local.image_count);

	// Per the XR_META_environment_depth spec, the Vulkan swapchain format is
	// VK_FORMAT_D16_UNORM and it is a 2D array texture with 2 layers
	// (layer 0 = left eye, layer 1 = right eye)
	const int32_t width       = (int32_t)local.swapchain_state.width;
	const int32_t height      = (int32_t)local.swapchain_state.height;
	const int32_t array_count = 2;
	const int64_t native_fmt  = tex_fmt_to_native(tex_format_depth16);

	log_infof("XR_META_environment_depth: swapchain images=%u size=%dx%d fmt=D16_UNORM(native=%lld) layers=%d",
		local.image_count, width, height, (long long)native_fmt, array_count);

	for (uint32_t i = 0; i < local.image_count; i++) {
		local.textures[i] = tex_create(tex_type_image_nomips, tex_format_depth16);
		if (local.textures[i] == nullptr) {
			log_warn("XR_META_environment_depth: Failed to create texture for depth swapchain image.");
			xr_ext_meta_environment_depth_destroy();
			return xr_system_fail;
		}

		void* image_ptr = (void*)(uintptr_t)local.images[i].image;
		tex_set_surface(local.textures[i], image_ptr, tex_type_image_nomips, native_fmt, width, height, array_count, 1, false);
		if (tex_asset_state(local.textures[i]) < asset_state_loaded) {
			log_warnf("XR_META_environment_depth: Failed to wrap depth image %u (VkImage=%p). Check format/layer compatibility.", i, image_ptr);
			xr_ext_meta_environment_depth_destroy();
			return xr_system_fail;
		}
	}

	local.available = true;
	return xr_system_succeed;
}

///////////////////////////////////////////

void xr_ext_meta_environment_depth_destroy() {
	if (local.running && local.provider != XR_NULL_HANDLE && xrStopEnvironmentDepthProviderMETA != nullptr) {
		xrStopEnvironmentDepthProviderMETA(local.provider);
	}
	local.running = false;

	if (local.textures != nullptr) {
		for (uint32_t i = 0; i < local.image_count; i++) {
			if (local.textures[i] != nullptr)
				tex_release(local.textures[i]);
		}
		sk_free(local.textures);
		local.textures = nullptr;
	}
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

	if (local.latest_mutex != nullptr) {
		ft_mutex_lock(local.latest_mutex);
		local.latest_frame = {};
		local.has_latest_frame = false;
		ft_mutex_unlock(local.latest_mutex);
	}
}

///////////////////////////////////////////

void xr_ext_meta_environment_depth_shutdown(void*) {
	xr_ext_meta_environment_depth_destroy();
	ft_mutex_destroy(&local.latest_mutex);
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

bool xr_ext_meta_environment_depth_supports_hand_removal() {
	return local.supports_hand_removal;
}

///////////////////////////////////////////

bool xr_ext_meta_environment_depth_set_hand_removal(bool32_t enabled) {
	if (!local.available || local.provider == XR_NULL_HANDLE || !local.supports_hand_removal || xrSetEnvironmentDepthHandRemovalMETA == nullptr)
		return false;

	XrEnvironmentDepthHandRemovalSetInfoMETA hand_removal = { XR_TYPE_ENVIRONMENT_DEPTH_HAND_REMOVAL_SET_INFO_META };
	hand_removal.enabled = enabled ? XR_TRUE : XR_FALSE;

	XrResult result = xrSetEnvironmentDepthHandRemovalMETA(local.provider, &hand_removal);
	if (XR_FAILED(result)) {
		log_warnf("XR_META_environment_depth: xrSetEnvironmentDepthHandRemovalMETA failed: [%s]", openxr_string(result));
		return false;
	}

	local.hand_removal = enabled != 0;
	return true;
}

///////////////////////////////////////////

bool xr_ext_meta_environment_depth_start() {
	if (!local.available || local.provider == XR_NULL_HANDLE || xrStartEnvironmentDepthProviderMETA == nullptr)
		return false;
	if (local.running)
		return true;

	XrResult result = xrStartEnvironmentDepthProviderMETA(local.provider);
	if (XR_FAILED(result)) {
		log_warnf("XR_META_environment_depth: xrStartEnvironmentDepthProviderMETA failed: [%s]", openxr_string(result));
		return false;
	}

	local.running = true;
	return true;
}

///////////////////////////////////////////

void xr_ext_meta_environment_depth_stop() {
	if (!local.running || local.provider == XR_NULL_HANDLE || xrStopEnvironmentDepthProviderMETA == nullptr)
		return;

	XrResult result = xrStopEnvironmentDepthProviderMETA(local.provider);
	if (XR_FAILED(result)) {
		log_warnf("XR_META_environment_depth: xrStopEnvironmentDepthProviderMETA failed: [%s]", openxr_string(result));
	}
	local.running = false;
}

///////////////////////////////////////////

void xr_ext_meta_environment_depth_update_frame(XrTime display_time) {
	if (!local.running || !local.available || local.provider == XR_NULL_HANDLE || xrAcquireEnvironmentDepthImageMETA == nullptr || xr_app_space == XR_NULL_HANDLE)
		return;

	XrEnvironmentDepthImageAcquireInfoMETA acquire_info = { XR_TYPE_ENVIRONMENT_DEPTH_IMAGE_ACQUIRE_INFO_META };
	acquire_info.space       = xr_app_space;
	acquire_info.displayTime = display_time;

	XrEnvironmentDepthImageMETA image_info = { XR_TYPE_ENVIRONMENT_DEPTH_IMAGE_META };
	image_info.views[0] = { XR_TYPE_ENVIRONMENT_DEPTH_IMAGE_VIEW_META };
	image_info.views[1] = { XR_TYPE_ENVIRONMENT_DEPTH_IMAGE_VIEW_META };

	XrResult result = xrAcquireEnvironmentDepthImageMETA(local.provider, &acquire_info, &image_info);
	if (result == XR_ENVIRONMENT_DEPTH_NOT_AVAILABLE_META)
		return;
	if (XR_FAILED(result)) {
		log_warnf("XR_META_environment_depth: xrAcquireEnvironmentDepthImageMETA failed: [%s]", openxr_string(result));
		return;
	}

	if (image_info.swapchainIndex >= local.image_count || local.textures == nullptr || local.textures[image_info.swapchainIndex] == nullptr)
		return;

	environment_depth_frame_t frame = {};
	frame.texture         = local.textures[image_info.swapchainIndex];
	frame.display_time    = display_time;
	frame.width           = local.swapchain_state.width;
	frame.height          = local.swapchain_state.height;
	frame.near_z          = image_info.nearZ;
	frame.far_z           = image_info.farZ;
	frame.left.pose       = xr_to_pose(image_info.views[0].pose);
	frame.left.fov        = xr_to_fov (image_info.views[0].fov );
	frame.right.pose      = xr_to_pose(image_info.views[1].pose);
	frame.right.fov       = xr_to_fov (image_info.views[1].fov );

	ft_mutex_lock(local.latest_mutex);
	local.latest_frame = frame;
	local.has_latest_frame = true;
	ft_mutex_unlock(local.latest_mutex);
}

///////////////////////////////////////////

bool xr_ext_meta_environment_depth_try_get_latest(environment_depth_frame_t* out_frame) {
	if (out_frame == nullptr || !local.available)
		return false;

	environment_depth_frame_t frame = {};
	bool has_frame = false;

	ft_mutex_lock(local.latest_mutex);
	has_frame = local.has_latest_frame;
	frame     = local.latest_frame;
	if (has_frame && frame.texture != nullptr)
		tex_addref(frame.texture);
	ft_mutex_unlock(local.latest_mutex);

	if (!has_frame)
		return false;

	*out_frame = frame;
	return true;
}

///////////////////////////////////////////

} // namespace sk