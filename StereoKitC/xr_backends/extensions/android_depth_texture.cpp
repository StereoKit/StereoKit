/* SPDX-License-Identifier: MIT */
/* The authors below grant copyright rights under the MIT license:
 * Copyright (c) 2026 Nick Klingensmith
 */

#include "android_depth_texture.h"

#include "../../asset_types/texture.h"
#include "../../log.h"
#include "../../sk_math.h"
#include "../../sk_memory.h"
#include "ext_management.h"

#include <stdint.h>
#include <string.h>

// XR_ANDROID_depth_texture (extension #703, spec v1). The bundled OpenXR
// headers predate this Android XR vendor extension, so the relevant handles,
// constants, structs, and function pointers are defined here, matching
// third_party/xr_android/xr_android_depth_texture.h from the Android XR SDK.
#ifndef XR_ANDROID_depth_texture
#define XR_ANDROID_depth_texture 1

XR_DEFINE_HANDLE(XrDepthSwapchainANDROID)

#define XR_ANDROID_DEPTH_TEXTURE_EXTENSION_NAME "XR_ANDROID_depth_texture"

#define XR_TYPE_DEPTH_SWAPCHAIN_CREATE_INFO_ANDROID      ((XrStructureType) 1000702000U)
#define XR_TYPE_DEPTH_VIEW_ANDROID                       ((XrStructureType) 1000702001U)
#define XR_TYPE_DEPTH_ACQUIRE_INFO_ANDROID               ((XrStructureType) 1000702002U)
#define XR_TYPE_DEPTH_ACQUIRE_RESULT_ANDROID             ((XrStructureType) 1000702003U)
#define XR_TYPE_SYSTEM_DEPTH_TRACKING_PROPERTIES_ANDROID ((XrStructureType) 1000702004U)
#define XR_TYPE_DEPTH_SWAPCHAIN_IMAGE_ANDROID            ((XrStructureType) 1000702005U)

#define XR_ERROR_DEPTH_NOT_AVAILABLE_ANDROID             ((XrResult) -1000702000)

typedef enum XrDepthCameraResolutionANDROID {
	XR_DEPTH_CAMERA_RESOLUTION_80x80_ANDROID    = 0,
	XR_DEPTH_CAMERA_RESOLUTION_160x160_ANDROID  = 1,
	XR_DEPTH_CAMERA_RESOLUTION_320x320_ANDROID  = 2,
	XR_DEPTH_CAMERA_RESOLUTION_MAX_ENUM_ANDROID = 0x7FFFFFFF
} XrDepthCameraResolutionANDROID;

typedef XrFlags64 XrDepthSwapchainCreateFlagsANDROID;
static const XrDepthSwapchainCreateFlagsANDROID XR_DEPTH_SWAPCHAIN_CREATE_SMOOTH_DEPTH_IMAGE_BIT_ANDROID      = 0x00000001;
static const XrDepthSwapchainCreateFlagsANDROID XR_DEPTH_SWAPCHAIN_CREATE_SMOOTH_CONFIDENCE_IMAGE_BIT_ANDROID = 0x00000002;
static const XrDepthSwapchainCreateFlagsANDROID XR_DEPTH_SWAPCHAIN_CREATE_RAW_DEPTH_IMAGE_BIT_ANDROID         = 0x00000004;
static const XrDepthSwapchainCreateFlagsANDROID XR_DEPTH_SWAPCHAIN_CREATE_RAW_CONFIDENCE_IMAGE_BIT_ANDROID    = 0x00000008;

typedef struct XrSystemDepthTrackingPropertiesANDROID {
	XrStructureType type;
	const void* XR_MAY_ALIAS next;
	XrBool32 supportsDepthTracking;
} XrSystemDepthTrackingPropertiesANDROID;

typedef struct XrDepthSwapchainCreateInfoANDROID {
	XrStructureType type;
	const void* XR_MAY_ALIAS next;
	XrDepthCameraResolutionANDROID resolution;
	XrDepthSwapchainCreateFlagsANDROID createFlags;
} XrDepthSwapchainCreateInfoANDROID;

typedef struct XrDepthSwapchainImageANDROID {
	XrStructureType type;
	void* XR_MAY_ALIAS next;
	const float*   rawDepthImage;
	const uint8_t* rawDepthConfidenceImage;
	const float*   smoothDepthImage;
	const uint8_t* smoothDepthConfidenceImage;
} XrDepthSwapchainImageANDROID;

typedef struct XrDepthAcquireInfoANDROID {
	XrStructureType type;
	const void* XR_MAY_ALIAS next;
	XrSpace space;
	XrTime  displayTime;
} XrDepthAcquireInfoANDROID;

typedef struct XrDepthViewANDROID {
	XrStructureType type;
	const void* XR_MAY_ALIAS next;
	XrFovf  fov;
	XrPosef pose;
} XrDepthViewANDROID;

typedef struct XrDepthAcquireResultANDROID {
	XrStructureType type;
	const void* XR_MAY_ALIAS next;
	uint32_t           acquiredIndex;
	XrTime             exposureTimestamp;
	XrDepthViewANDROID views[2];
} XrDepthAcquireResultANDROID;

typedef XrResult (XRAPI_PTR *PFN_xrEnumerateDepthResolutionsANDROID)    (XrSession session, uint32_t resolutionCapacityInput, uint32_t* resolutionCountOutput, XrDepthCameraResolutionANDROID* resolutions);
typedef XrResult (XRAPI_PTR *PFN_xrCreateDepthSwapchainANDROID)         (XrSession session, const XrDepthSwapchainCreateInfoANDROID* createInfo, XrDepthSwapchainANDROID* swapchain);
typedef XrResult (XRAPI_PTR *PFN_xrDestroyDepthSwapchainANDROID)        (XrDepthSwapchainANDROID swapchain);
typedef XrResult (XRAPI_PTR *PFN_xrEnumerateDepthSwapchainImagesANDROID)(XrDepthSwapchainANDROID depthSwapchain, uint32_t depthImageCapacityInput, uint32_t* depthImageCountOutput, XrDepthSwapchainImageANDROID* depthImages);
typedef XrResult (XRAPI_PTR *PFN_xrAcquireDepthSwapchainImagesANDROID)  (XrDepthSwapchainANDROID depthSwapchain, const XrDepthAcquireInfoANDROID* acquireInfo, XrDepthAcquireResultANDROID* acquireResult);
#endif // XR_ANDROID_depth_texture

#define XR_ANDROID_DEPTH_TEXTURE_FUNCTIONS(X) \
	X(xrEnumerateDepthResolutionsANDROID)     \
	X(xrCreateDepthSwapchainANDROID)          \
	X(xrDestroyDepthSwapchainANDROID)         \
	X(xrEnumerateDepthSwapchainImagesANDROID) \
	X(xrAcquireDepthSwapchainImagesANDROID)
OPENXR_DEFINE_FN_STATIC(XR_ANDROID_DEPTH_TEXTURE_FUNCTIONS);

namespace sk {

///////////////////////////////////////////

typedef struct xr_android_depth_state_t {
	bool                          available;
	bool                          running;
	sensor_depth_caps_            active_flags;
	bool                          pending_recreate;
	XrDepthSwapchainANDROID       swapchain;
	XrDepthCameraResolutionANDROID resolution;
	int32_t                       width;
	int32_t                       height;
	uint32_t                      image_count;
	XrDepthSwapchainImageANDROID* images;
	int32_t                       resolution_pref;
	int32_t                       resolutions_px[8];
	int32_t                       resolutions_count;
	void*                         image_buffers   [4];
	size_t                        image_buffer_len[4];
	uint32_t                      available_images;
	tex_t                         depth_tex;
	bool                          tex_wanted;
	uint64_t                      frame_seq;
	uint64_t                      tex_seq;
	bool                          has_latest_frame;
	sensor_depth_frame_t          latest_frame;
	XrTime                        last_stop_time;
} xr_android_depth_state_t;
static xr_android_depth_state_t local = {};

// Per-pixel element size of an image kind: confidence is uint8, depth is float.
inline size_t android_depth_image_elem_size(sensor_depth_image_ image) {
	return (image == sensor_depth_image_smooth_confidence || image == sensor_depth_image_raw_confidence)
		? sizeof(uint8_t) : sizeof(float);
}

///////////////////////////////////////////

xr_system_ xr_ext_android_depth_texture_initialize(void*);
void       xr_ext_android_depth_texture_shutdown  (void*);
void       xr_ext_android_depth_texture_destroy   ();
static void android_depth_free_buffers();

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

inline int32_t resolution_to_pixels(XrDepthCameraResolutionANDROID resolution) {
	switch (resolution) {
	case XR_DEPTH_CAMERA_RESOLUTION_80x80_ANDROID:   return 80;
	case XR_DEPTH_CAMERA_RESOLUTION_160x160_ANDROID: return 160;
	case XR_DEPTH_CAMERA_RESOLUTION_320x320_ANDROID: return 320;
	default:                                         return 0;
	}
}

///////////////////////////////////////////

void xr_ext_android_depth_texture_register() {
	local = {};

	xr_system_t sys = {};
	sys.request_exts[sys.request_ext_count++] = XR_ANDROID_DEPTH_TEXTURE_EXTENSION_NAME;
	sys.evt_initialize = { xr_ext_android_depth_texture_initialize };
	sys.evt_shutdown   = { xr_ext_android_depth_texture_shutdown };
	ext_management_sys_register(sys);
}

///////////////////////////////////////////

xr_system_ xr_ext_android_depth_texture_initialize(void*) {
	if (!backend_openxr_ext_enabled(XR_ANDROID_DEPTH_TEXTURE_EXTENSION_NAME))
		return xr_system_fail;

	OPENXR_LOAD_FN_RETURN(XR_ANDROID_DEPTH_TEXTURE_FUNCTIONS, xr_system_fail);

	XrSystemDepthTrackingPropertiesANDROID properties_depth = { XR_TYPE_SYSTEM_DEPTH_TRACKING_PROPERTIES_ANDROID };
	XrSystemProperties                     properties       = { XR_TYPE_SYSTEM_PROPERTIES };
	properties.next = &properties_depth;

	XrResult result = xrGetSystemProperties(xr_instance, xr_system_id, &properties);
	if (XR_FAILED(result)) {
		log_warnf("XR_ANDROID_depth_texture: xrGetSystemProperties failed: [%s]", openxr_string(result));
		return xr_system_fail;
	}
	if (properties_depth.supportsDepthTracking == XR_FALSE)
		return xr_system_fail;

	local.available               = true;
	return xr_system_succeed;
}

///////////////////////////////////////////

void xr_ext_android_depth_texture_destroy() {
	local.running          = false;
	local.active_flags     = sensor_depth_caps_none;
	local.pending_recreate = false;

	local.latest_frame     = {};
	local.has_latest_frame = false;
	local.available_images = 0;

	android_depth_free_buffers();

	local.tex_wanted = false;
	local.frame_seq  = 0;
	local.tex_seq    = 0;

	if (local.images != nullptr) {
		sk_free(local.images);
		local.images = nullptr;
	}
	local.image_count = 0;

	if (local.swapchain != XR_NULL_HANDLE && xrDestroyDepthSwapchainANDROID != nullptr) {
		xrDestroyDepthSwapchainANDROID(local.swapchain);
		local.swapchain = XR_NULL_HANDLE;
	}
	local.width  = 0;
	local.height = 0;
}

///////////////////////////////////////////

void xr_ext_android_depth_texture_shutdown(void*) {
	xr_ext_android_depth_texture_destroy();
	if (local.depth_tex != nullptr) {
		tex_release(local.depth_tex);
		local.depth_tex = nullptr;
	}
	OPENXR_CLEAR_FN(XR_ANDROID_DEPTH_TEXTURE_FUNCTIONS);
	local = {};
}

///////////////////////////////////////////

bool xr_ext_android_depth_texture_available() {
	return local.available;
}

///////////////////////////////////////////

bool xr_ext_android_depth_texture_running() {
	return local.running;
}

///////////////////////////////////////////

sensor_depth_caps_ xr_ext_android_depth_texture_get_capabilities() {
	// Raw and/or smooth depth plus their confidence, all from one frame.
	return (sensor_depth_caps_)(sensor_depth_caps_raw_depth | sensor_depth_caps_smooth_depth | sensor_depth_caps_confidence);
}

///////////////////////////////////////////

static void android_depth_free_buffers() {
	for (int32_t i = 0; i < 4; i++) {
		if (local.image_buffers[i] != nullptr) { sk_free(local.image_buffers[i]); local.image_buffers[i] = nullptr; }
		local.image_buffer_len[i] = 0;
	}
}

// Maps the active caps to the images to request + the swapchain create flags,
// defaulting to smooth depth. Returns a (1<<sensor_depth_image_) mask.
static uint32_t android_depth_wanted_images(sensor_depth_caps_ flags, XrDepthSwapchainCreateFlagsANDROID* out_create_flags) {
	bool want_raw    = (flags & sensor_depth_caps_raw_depth)    != 0;
	bool want_smooth = (flags & sensor_depth_caps_smooth_depth) != 0 || !want_raw;
	bool want_conf   = (flags & sensor_depth_caps_confidence)   != 0;

	uint32_t                           images = 0;
	XrDepthSwapchainCreateFlagsANDROID create = 0;
	if (want_smooth)              { images |= 1u << sensor_depth_image_smooth_depth;      create |= XR_DEPTH_SWAPCHAIN_CREATE_SMOOTH_DEPTH_IMAGE_BIT_ANDROID; }
	if (want_raw)                 { images |= 1u << sensor_depth_image_raw_depth;         create |= XR_DEPTH_SWAPCHAIN_CREATE_RAW_DEPTH_IMAGE_BIT_ANDROID; }
	if (want_smooth && want_conf) { images |= 1u << sensor_depth_image_smooth_confidence; create |= XR_DEPTH_SWAPCHAIN_CREATE_SMOOTH_CONFIDENCE_IMAGE_BIT_ANDROID; }
	if (want_raw    && want_conf) { images |= 1u << sensor_depth_image_raw_confidence;    create |= XR_DEPTH_SWAPCHAIN_CREATE_RAW_CONFIDENCE_IMAGE_BIT_ANDROID; }
	if (out_create_flags) *out_create_flags = create;
	return images;
}

// Creates (or recreates) the depth swapchain, image array, and a CPU buffer per
// requested image. Releases any existing swapchain resources first.
static bool android_depth_build_swapchain() {
	android_depth_free_buffers();
	if (local.images != nullptr) { sk_free(local.images); local.images = nullptr; }
	local.image_count      = 0;
	if (local.swapchain != XR_NULL_HANDLE && xrDestroyDepthSwapchainANDROID != nullptr) {
		xrDestroyDepthSwapchainANDROID(local.swapchain);
		local.swapchain = XR_NULL_HANDLE;
	}
	local.has_latest_frame = false;
	local.available_images = 0;

	XrDepthSwapchainCreateFlagsANDROID create_flags = 0;
	uint32_t                           wanted       = android_depth_wanted_images(local.active_flags, &create_flags);

	XrDepthSwapchainCreateInfoANDROID swapchain_info = { XR_TYPE_DEPTH_SWAPCHAIN_CREATE_INFO_ANDROID };
	swapchain_info.resolution  = local.resolution;
	swapchain_info.createFlags = create_flags;

	XrResult result = xrCreateDepthSwapchainANDROID(xr_session, &swapchain_info, &local.swapchain);
	if (XR_FAILED(result)) {
		log_warnf("XR_ANDROID_depth_texture: xrCreateDepthSwapchainANDROID failed: [%s] (is SCENE_UNDERSTANDING_FINE granted?)", openxr_string(result));
		return false;
	}

	result = xrEnumerateDepthSwapchainImagesANDROID(local.swapchain, 0, &local.image_count, nullptr);
	if (XR_FAILED(result) || local.image_count == 0) {
		log_warnf("XR_ANDROID_depth_texture: xrEnumerateDepthSwapchainImagesANDROID(count) failed: [%s]", openxr_string(result));
		return false;
	}

	local.images = sk_malloc_t(XrDepthSwapchainImageANDROID, local.image_count);
	if (local.images == nullptr) {
		log_warn("XR_ANDROID_depth_texture: failed to allocate swapchain image array.");
		return false;
	}
	for (uint32_t i = 0; i < local.image_count; i++) {
		local.images[i] = { XR_TYPE_DEPTH_SWAPCHAIN_IMAGE_ANDROID };
	}

	result = xrEnumerateDepthSwapchainImagesANDROID(local.swapchain, local.image_count, &local.image_count, local.images);
	if (XR_FAILED(result)) {
		log_warnf("XR_ANDROID_depth_texture: xrEnumerateDepthSwapchainImagesANDROID(images) failed: [%s]", openxr_string(result));
		return false;
	}

	// One CPU buffer per requested image, holding both views.
	size_t pixels = (size_t)local.width * local.height * 2;
	for (int32_t i = 0; i < 4; i++) {
		if ((wanted & (1u << i)) == 0) continue;
		size_t bytes              = pixels * android_depth_image_elem_size((sensor_depth_image_)i);
		local.image_buffers[i]    = sk_malloc_t(uint8_t, bytes);
		local.image_buffer_len[i] = bytes;
		if (local.image_buffers[i] == nullptr) {
			log_warn("XR_ANDROID_depth_texture: failed to allocate depth image buffer.");
			return false;
		}
	}

	log_infof("XR_ANDROID_depth_texture: swapchain ready, images=%u size=%dx%d requested=0x%X views=2", local.image_count, local.width, local.height, wanted);
	return true;
}

///////////////////////////////////////////

bool xr_ext_android_depth_texture_start(sensor_depth_caps_ flags) {
	if (!local.available || xrCreateDepthSwapchainANDROID == nullptr)
		return false;
	if (local.running)
		return true;

	// SCENE_UNDERSTANDING_FINE must be granted before creating the swapchain.
	// Request it if it's not yet granted and return false; call start() again
	// once the async grant resolves.
	permission_state_ scene_perm = permission_state(permission_type_scene_fine);
	if (scene_perm != permission_state_granted) {
		if (scene_perm == permission_state_capable)
			permission_request(permission_type_scene_fine);
		else
			log_warn("XR_ANDROID_depth_texture: SCENE_UNDERSTANDING_FINE is not in the app manifest; depth cannot start.");
		return false;
	}

	// Use the app's preferred resolution if available, else the highest.
	XrDepthCameraResolutionANDROID resolutions[8] = {};
	uint32_t                       resolution_count = 0;
	XrResult result = xrEnumerateDepthResolutionsANDROID(xr_session, 0, &resolution_count, nullptr);
	if (XR_FAILED(result) || resolution_count == 0) {
		log_warnf("XR_ANDROID_depth_texture: xrEnumerateDepthResolutionsANDROID(count) failed: [%s]", openxr_string(result));
		return false;
	}
	if (resolution_count > 8) resolution_count = 8;
	result = xrEnumerateDepthResolutionsANDROID(xr_session, resolution_count, &resolution_count, resolutions);
	if (XR_FAILED(result)) {
		log_warnf("XR_ANDROID_depth_texture: xrEnumerateDepthResolutionsANDROID(values) failed: [%s]", openxr_string(result));
		return false;
	}

	XrDepthCameraResolutionANDROID chosen = resolutions[0];
	int32_t                        chosen_px = resolution_to_pixels(chosen);
	for (uint32_t i = 0; i < resolution_count; i++) {
		int32_t px = resolution_to_pixels(resolutions[i]);
		if (local.resolution_pref > 0 && px == local.resolution_pref) { chosen = resolutions[i]; chosen_px = px; break; }
		if (px > chosen_px) { chosen = resolutions[i]; chosen_px = px; }
	}
	if (chosen_px == 0) {
		log_warn("XR_ANDROID_depth_texture: no usable depth resolution reported.");
		return false;
	}
	local.resolution = chosen;
	local.width      = chosen_px;
	local.height     = chosen_px;
	local.active_flags     = flags;
	local.pending_recreate = false;

	if (!android_depth_build_swapchain()) {
		xr_ext_android_depth_texture_destroy();
		return false;
	}

	local.running = true;
	return true;
}

///////////////////////////////////////////

void xr_ext_android_depth_texture_stop() {
	if (!local.running)
		return;

	if (local.has_latest_frame) {
		XrTime frame_time    = local.latest_frame.capture_time != 0 ? local.latest_frame.capture_time : local.latest_frame.display_time;
		local.last_stop_time = frame_time;
	}

	xr_ext_android_depth_texture_destroy();
}

///////////////////////////////////////////

bool xr_ext_android_depth_texture_set_caps(sensor_depth_caps_ flags) {
	if (!local.running) return false;

	// A change to which images are requested needs a swapchain recreate.
	const sensor_depth_caps_ image_caps = (sensor_depth_caps_)(sensor_depth_caps_raw_depth | sensor_depth_caps_smooth_depth | sensor_depth_caps_confidence);
	if (((flags ^ local.active_flags) & image_caps) != 0) {
		local.pending_recreate = true;
		log_info("XR_ANDROID_depth_texture: requested depth images changed (recreate pending)");
	}
	local.active_flags = flags;
	return true;
}

///////////////////////////////////////////

// The primary depth image: smooth if present this frame, otherwise raw.
static sensor_depth_image_ android_depth_primary_image() {
	return (local.available_images & (1u << sensor_depth_image_smooth_depth)) != 0
		? sensor_depth_image_smooth_depth
		: sensor_depth_image_raw_depth;
}

// Uploads the latest primary-depth buffer into depth_tex when a newer frame is ready
// (two-layer R32, one layer per eye). No-op until the texture has been requested.
static void android_depth_upload_tex() {
	if (local.depth_tex == nullptr || !local.has_latest_frame || local.tex_seq == local.frame_seq)
		return;
	sensor_depth_image_ primary = android_depth_primary_image();
	if (local.image_buffers[primary] == nullptr)
		return;
	float* buf       = (float*)local.image_buffers[primary];
	void*  layers[2] = { buf, buf + (size_t)local.width * local.height };
	tex_set_color_arr(local.depth_tex, local.width, local.height, layers, 2);
	local.tex_seq = local.frame_seq;
}

tex_t xr_ext_android_depth_texture_get_texture() {
	// Android delivers depth as CPU buffers; the GPU texture is provided lazily so only
	// callers that want one pay the upload (CPU consumers use try_get_data directly). Once
	// requested, update_frame keeps it current, so the caller can fetch the handle once.
	if (!local.has_latest_frame || local.image_buffers[android_depth_primary_image()] == nullptr)
		return nullptr;

	if (local.depth_tex == nullptr) {
		local.depth_tex = tex_create(tex_type_image_nomips | tex_type_dynamic, tex_format_r32);
		tex_set_id     (local.depth_tex, "sk/sensor_depth/android");
		tex_set_sample (local.depth_tex, tex_sample_point);
		tex_set_address(local.depth_tex, tex_address_clamp);
	}
	local.tex_wanted = true;
	android_depth_upload_tex();
	return local.depth_tex;
}

///////////////////////////////////////////

bool xr_ext_android_depth_texture_try_get_latest(sensor_depth_frame_t* out_frame) {
	if (out_frame == nullptr || !local.available || !local.has_latest_frame)
		return false;

	*out_frame = local.latest_frame;
	return true;
}

///////////////////////////////////////////

static bool android_depth_copy_image(sensor_depth_image_ image, sensor_depth_frame_t* out_frame, void* out_data, size_t* out_data_size, int32_t view_index) {
	if (out_frame == nullptr || !local.running || !local.has_latest_frame
		|| (int32_t)image < 0 || (int32_t)image >= 4
		|| local.image_buffers[image] == nullptr
		|| (local.available_images & (1u << image)) == 0) {
		if (out_frame)     *out_frame     = {};
		if (out_data_size) *out_data_size = 0;
		return false;
	}

	size_t  elem        = android_depth_image_elem_size(image);
	int32_t layer_size  = local.width * local.height;
	int32_t layer_count = view_index < 0 ? 2 : 1;
	size_t  data_bytes  = (size_t)layer_size * layer_count * elem;
	if (out_data_size) *out_data_size = data_bytes;

	if (out_data != nullptr) {
		size_t src_offset = (size_t)(view_index == 1 ? layer_size : 0) * elem;
		memcpy(out_data, (uint8_t*)local.image_buffers[image] + src_offset, data_bytes);
	}

	*out_frame = local.latest_frame;
	return true;
}

bool xr_ext_android_depth_texture_try_get_image(sensor_depth_image_ image, sensor_depth_frame_t* out_frame, void* out_data, size_t* out_data_size, int32_t view_index) {
	return android_depth_copy_image(image, out_frame, out_data, out_data_size, view_index);
}

void xr_ext_android_depth_texture_get_resolutions(const int32_t** out_arr_resolutions, int32_t* out_count) {
	local.resolutions_count = 0;
	if (local.available && xrEnumerateDepthResolutionsANDROID != nullptr) {
		XrDepthCameraResolutionANDROID resolutions[8] = {};
		uint32_t                       count = 0;
		if (XR_SUCCEEDED(xrEnumerateDepthResolutionsANDROID(xr_session, 0, &count, nullptr)) && count > 0) {
			if (count > 8) count = 8;
			if (XR_SUCCEEDED(xrEnumerateDepthResolutionsANDROID(xr_session, count, &count, resolutions))) {
				for (uint32_t i = 0; i < count; i++)
					local.resolutions_px[i] = resolution_to_pixels(resolutions[i]);
				local.resolutions_count = (int32_t)count;
			}
		}
	}
	if (out_arr_resolutions) *out_arr_resolutions = local.resolutions_px;
	if (out_count)           *out_count           = local.resolutions_count;
}

void xr_ext_android_depth_texture_set_resolution(int32_t resolution) {
	local.resolution_pref = resolution;
}

///////////////////////////////////////////

void xr_ext_android_depth_texture_update_frame(XrTime display_time) {
	if (!local.running || !local.available || xrAcquireDepthSwapchainImagesANDROID == nullptr || xr_app_space == XR_NULL_HANDLE)
		return;

	// Apply a pending raw/smooth switch here (render thread, between acquires).
	if (local.pending_recreate) {
		local.pending_recreate = false;
		if (!android_depth_build_swapchain())
			log_warn("XR_ANDROID_depth_texture: depth source recreate failed.");
		return; // acquire from the new swapchain next frame
	}

	if (local.swapchain == XR_NULL_HANDLE)
		return;

	XrDepthAcquireInfoANDROID acquire_info = { XR_TYPE_DEPTH_ACQUIRE_INFO_ANDROID };
	acquire_info.space       = xr_app_space;
	acquire_info.displayTime = display_time;

	XrDepthAcquireResultANDROID acquire_result = { XR_TYPE_DEPTH_ACQUIRE_RESULT_ANDROID };
	acquire_result.views[0] = { XR_TYPE_DEPTH_VIEW_ANDROID };
	acquire_result.views[1] = { XR_TYPE_DEPTH_VIEW_ANDROID };

	XrResult result = xrAcquireDepthSwapchainImagesANDROID(local.swapchain, &acquire_info, &acquire_result);
	if (result == XR_ERROR_DEPTH_NOT_AVAILABLE_ANDROID)
		return;
	if (XR_FAILED(result)) {
		log_warnf("XR_ANDROID_depth_texture: xrAcquireDepthSwapchainImagesANDROID failed: [%s]", openxr_string(result));
		return;
	}

	if (local.last_stop_time != 0) {
		XrTime frame_time = acquire_result.exposureTimestamp != 0 ? acquire_result.exposureTimestamp : display_time;
		if (frame_time <= local.last_stop_time)
			return;
		local.last_stop_time = 0;
	}

	if (acquire_result.acquiredIndex >= local.image_count || local.images == nullptr) {
		log_warnf("XR_ANDROID_depth_texture: invalid acquired index %u (image_count=%u)", acquire_result.acquiredIndex, local.image_count);
		return;
	}

	const XrDepthSwapchainImageANDROID& image = local.images[acquire_result.acquiredIndex];

	// Copy each requested image the runtime populated into its CPU buffer.
	const void* srcs[4] = {
		image.smoothDepthImage,
		image.rawDepthImage,
		image.smoothDepthConfidenceImage,
		image.rawDepthConfidenceImage,
	};
	uint32_t avail = 0;
	for (int32_t i = 0; i < 4; i++) {
		if (local.image_buffers[i] != nullptr && srcs[i] != nullptr) {
			memcpy(local.image_buffers[i], srcs[i], local.image_buffer_len[i]);
			avail |= 1u << i;
		}
	}
	if (avail == 0) {
		log_warn("XR_ANDROID_depth_texture: acquired frame has no requested image data.");
		return;
	}

	sensor_depth_frame_t frame = {};
	frame.display_time  = display_time;
	frame.capture_time  = acquire_result.exposureTimestamp;
	frame.width         = (uint32_t)local.width;
	frame.height        = (uint32_t)local.height;
	frame.near_z        = 0.0f;
	frame.far_z         = 0.0f;
	frame.views[0].pose = xr_to_pose(acquire_result.views[0].pose);
	frame.views[0].fov  = xr_to_fov (acquire_result.views[0].fov );
	frame.views[1].pose = xr_to_pose(acquire_result.views[1].pose);
	frame.views[1].fov  = xr_to_fov (acquire_result.views[1].fov );
	frame.depth_format     = sensor_depth_format_meters_r32;
	frame.storage          = sensor_depth_storage_cpu_buffer;
	frame.view_count       = 2;
	frame.available_images = avail;

	local.available_images = avail;
	local.latest_frame     = frame;
	local.has_latest_frame = true;
	local.frame_seq++;

	// Keep the GPU texture current once something has asked for it.
	if (local.tex_wanted)
		android_depth_upload_tex();
}

///////////////////////////////////////////

} // namespace sk
