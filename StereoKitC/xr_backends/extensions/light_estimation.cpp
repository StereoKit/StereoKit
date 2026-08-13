#include "../../stereokit.h"
#include "../openxr.h"
#include "ext_management.h"
#include "light_estimation.h"

#include "../../asset_types/texture.h"
#include "../../sk_memory.h"
#include "../../systems/lighting.h"

///////////////////////////////////////////
// XR_ANDROID_light_estimation_cubemap isn't in the Khronos OpenXR headers
// yet (extension 722, revision 1, 2025-12-05), so its definitions live here
// until the SDK catches up. The guard drops them once they arrive upstream.
///////////////////////////////////////////

#ifndef XR_ANDROID_light_estimation_cubemap
#define XR_ANDROID_light_estimation_cubemap 1
#define XR_ANDROID_light_estimation_cubemap_SPEC_VERSION 1
#define XR_ANDROID_LIGHT_ESTIMATION_CUBEMAP_EXTENSION_NAME "XR_ANDROID_light_estimation_cubemap"

#define XR_TYPE_SYSTEM_CUBEMAP_LIGHT_ESTIMATION_PROPERTIES_ANDROID ((XrStructureType) 1000721000U)
#define XR_TYPE_CUBEMAP_LIGHT_ESTIMATOR_CREATE_INFO_ANDROID        ((XrStructureType) 1000721001U)
#define XR_TYPE_CUBEMAP_LIGHTING_DATA_ANDROID                      ((XrStructureType) 1000721002U)

typedef enum XrCubemapLightingColorFormatANDROID {
	XR_CUBEMAP_LIGHTING_COLOR_FORMAT_R32G32B32_SFLOAT_ANDROID    = 1,
	XR_CUBEMAP_LIGHTING_COLOR_FORMAT_R32G32B32A32_SFLOAT_ANDROID = 2,
	XR_CUBEMAP_LIGHTING_COLOR_FORMAT_R16G16B16A16_SFLOAT_ANDROID = 3,
	XR_CUBEMAP_LIGHTING_COLOR_FORMAT_MAX_ENUM_ANDROID            = 0x7FFFFFFF
} XrCubemapLightingColorFormatANDROID;

typedef struct XrSystemCubemapLightEstimationPropertiesANDROID {
	XrStructureType type;
	void*           next;
	XrBool32        supportsCubemapLightEstimation;
} XrSystemCubemapLightEstimationPropertiesANDROID;

typedef struct XrCubemapLightEstimatorCreateInfoANDROID {
	XrStructureType                     type;
	const void*                         next;
	uint32_t                            cubemapResolution;
	XrCubemapLightingColorFormatANDROID colorFormat;
	XrBool32                            reproject;
} XrCubemapLightEstimatorCreateInfoANDROID;

typedef struct XrCubemapLightingDataANDROID {
	XrStructureType             type;
	void*                       next;
	XrLightEstimateStateANDROID state;
	uint32_t                    imageBufferSize;
	uint8_t*                    imageBufferRight;
	uint8_t*                    imageBufferLeft;
	uint8_t*                    imageBufferTop;
	uint8_t*                    imageBufferBottom;
	uint8_t*                    imageBufferFront;
	uint8_t*                    imageBufferBack;
	XrQuaternionf               rotation;
	XrTime                      centerExposureTime;
} XrCubemapLightingDataANDROID;

typedef XrResult (XRAPI_PTR *PFN_xrEnumerateCubemapLightingResolutionsANDROID)(
	XrInstance                           instance,
	XrSystemId                           systemId,
	uint32_t                             resolutionCapacityInput,
	uint32_t*                            resolutionCountOutput,
	uint32_t*                            resolutions);

typedef XrResult (XRAPI_PTR *PFN_xrEnumerateCubemapLightingColorFormatsANDROID)(
	XrInstance                           instance,
	XrSystemId                           systemId,
	uint32_t                             colorFormatCapacityInput,
	uint32_t*                            colorFormatCountOutput,
	XrCubemapLightingColorFormatANDROID* colorFormats);

#endif // XR_ANDROID_light_estimation_cubemap

///////////////////////////////////////////

#define XR_EXT_FUNCTIONS( X )  \
	X(xrCreateLightEstimatorANDROID)  \
	X(xrDestroyLightEstimatorANDROID) \
	X(xrGetLightEstimateANDROID)
OPENXR_DEFINE_FN_STATIC(XR_EXT_FUNCTIONS);

#define XR_EXT_CUBEMAP_FUNCTIONS( X )  \
	X(xrEnumerateCubemapLightingResolutionsANDROID) \
	X(xrEnumerateCubemapLightingColorFormatsANDROID)
OPENXR_DEFINE_FN_STATIC(XR_EXT_CUBEMAP_FUNCTIONS);

typedef struct xr_light_estimation_state_t {
	bool                      available;
	bool                      started;
	XrLightEstimatorANDROID   estimator;
	XrTime                    last_update;
	bool                      sh_updated;
	sk::spherical_harmonics_t sh_data;

	// XR_ANDROID_light_estimation_cubemap
	bool                                cubemap_available;
	bool                                cubemap_refused; // runtime rejected cubemaps
	bool                                cubemap_started;
	uint32_t                            cubemap_resolution;
	XrCubemapLightingColorFormatANDROID cubemap_format;
	sk::tex_format_                     cubemap_tex_format;
	uint32_t                            cubemap_face_bytes;
	uint8_t*                            cubemap_buffer; // 6 faces, +X -X +Y -Y +Z -Z
	uint8_t*                            cubemap_repack; // RGB32F -> RGBA32F
} xr_light_estimation_state_t;
static xr_light_estimation_state_t local = { };

///////////////////////////////////////////

namespace sk {

///////////////////////////////////////////

xr_system_ xr_ext_android_light_estimation_initialize(void*);
void       xr_ext_android_light_estimation_shutdown  (void*);
void       xr_ext_android_light_estimation_step_begin(void*);

///////////////////////////////////////////

void xr_ext_android_light_estimation_register() {
	xr_system_t sys = {};
	sys.request_exts    [sys.request_ext_count    ++] = XR_ANDROID_LIGHT_ESTIMATION_EXTENSION_NAME;
	// Cubemap estimates are additive, and SH estimation works without them.
	sys.request_opt_exts[sys.request_opt_ext_count++] = XR_ANDROID_LIGHT_ESTIMATION_CUBEMAP_EXTENSION_NAME;
	sys.evt_initialize = { xr_ext_android_light_estimation_initialize };
	sys.evt_shutdown   = { xr_ext_android_light_estimation_shutdown };
	sys.evt_step_begin = { xr_ext_android_light_estimation_step_begin };
	ext_management_sys_register(sys);
}

///////////////////////////////////////////

static bool _cubemap_load_fns() {
	OPENXR_LOAD_FN_RETURN(XR_EXT_CUBEMAP_FUNCTIONS, false);
	return true;
}

///////////////////////////////////////////

// Picks the format/resolution to request: prefer compact float formats, and
// the smallest resolution that still covers StereoKit's reflections.
static bool _cubemap_pick_settings() {
	// Formats in preference order. RGB32F has no GPU equivalent, and needs a
	// CPU repack to RGBA32F on upload.
	struct format_pref_t { XrCubemapLightingColorFormatANDROID xr_format; tex_format_ tex_format; };
	static const format_pref_t preference[] = {
		{ XR_CUBEMAP_LIGHTING_COLOR_FORMAT_R16G16B16A16_SFLOAT_ANDROID, tex_format_rgba64f },
		{ XR_CUBEMAP_LIGHTING_COLOR_FORMAT_R32G32B32A32_SFLOAT_ANDROID, tex_format_rgba128 },
		{ XR_CUBEMAP_LIGHTING_COLOR_FORMAT_R32G32B32_SFLOAT_ANDROID,    tex_format_rgba128 },
	};

	uint32_t format_count = 0;
	if (XR_FAILED(xrEnumerateCubemapLightingColorFormatsANDROID(xr_instance, xr_system_id, 0, &format_count, nullptr)) || format_count == 0)
		return false;
	XrCubemapLightingColorFormatANDROID* formats = sk_malloc_t(XrCubemapLightingColorFormatANDROID, format_count);
	XrResult result = xrEnumerateCubemapLightingColorFormatsANDROID(xr_instance, xr_system_id, format_count, &format_count, formats);

	local.cubemap_format = XR_CUBEMAP_LIGHTING_COLOR_FORMAT_MAX_ENUM_ANDROID;
	if (XR_SUCCEEDED(result)) {
		for (int32_t p = 0; p < (int32_t)_countof(preference) && local.cubemap_format == XR_CUBEMAP_LIGHTING_COLOR_FORMAT_MAX_ENUM_ANDROID; p++) {
			for (uint32_t i = 0; i < format_count; i++) {
				if (formats[i] != preference[p].xr_format) continue;
				local.cubemap_format     = preference[p].xr_format;
				local.cubemap_tex_format = preference[p].tex_format;
				break;
			}
		}
	}
	sk_free(formats);
	if (local.cubemap_format == XR_CUBEMAP_LIGHTING_COLOR_FORMAT_MAX_ENUM_ANDROID)
		return false;

	uint32_t resolution_count = 0;
	if (XR_FAILED(xrEnumerateCubemapLightingResolutionsANDROID(xr_instance, xr_system_id, 0, &resolution_count, nullptr)) || resolution_count == 0)
		return false;
	uint32_t* resolutions = sk_malloc_t(uint32_t, resolution_count);
	result = xrEnumerateCubemapLightingResolutionsANDROID(xr_instance, xr_system_id, resolution_count, &resolution_count, resolutions);

	// Smallest resolution that's >= the reflection size keeps upload and
	// convolution costs down; if none is, take the largest offered.
	uint32_t best = 0;
	if (XR_SUCCEEDED(result)) {
		for (uint32_t i = 0; i < resolution_count; i++) {
			uint32_t r = resolutions[i];
			bool covers      = r    >= SK_LIGHTING_REFLECTION_SIZE;
			bool best_covers = best >= SK_LIGHTING_REFLECTION_SIZE;
			if (best == 0 || (covers && (!best_covers || r < best)) || (!covers && !best_covers && r > best))
				best = r;
		}
	}
	sk_free(resolutions);
	if (best == 0) return false;

	local.cubemap_resolution = best;
	return true;
}

///////////////////////////////////////////

xr_system_ xr_ext_android_light_estimation_initialize(void*) {
	if (!backend_openxr_ext_enabled(XR_ANDROID_LIGHT_ESTIMATION_EXTENSION_NAME))
		return xr_system_fail;

	OPENXR_LOAD_FN_RETURN(XR_EXT_FUNCTIONS, xr_system_fail);

	// Check the system properties, see if we can actually do light estimation
	XrSystemProperties                              properties         = { XR_TYPE_SYSTEM_PROPERTIES };
	XrSystemLightEstimationPropertiesANDROID        properties_light   = { (XrStructureType)XR_TYPE_SYSTEM_LIGHT_ESTIMATION_PROPERTIES_ANDROID };
	XrSystemCubemapLightEstimationPropertiesANDROID properties_cubemap = { (XrStructureType)XR_TYPE_SYSTEM_CUBEMAP_LIGHT_ESTIMATION_PROPERTIES_ANDROID };
	properties.next = &properties_light;
	if (backend_openxr_ext_enabled(XR_ANDROID_LIGHT_ESTIMATION_CUBEMAP_EXTENSION_NAME))
		properties_light.next = &properties_cubemap;
	XrResult result = xrGetSystemProperties(xr_instance, xr_system_id, &properties);
	if (XR_FAILED(result)) {
		log_infof("%s: [%s]", "xrGetSystemProperties", openxr_string(result));
		return xr_system_fail;
	}
	if (properties_light.supportsLightEstimation == false) {
		log_diag("Light estimation: extension present, but the system doesn't support it.");
		return xr_system_fail;
	}

	// NOTE: the scene permission is deliberately NOT checked here! This runs
	// inside Platform init, before the Permission system, so it would always
	// read 'unknown'. xr_ext_android_light_estimation_available() checks it.

	// Cubemap estimates are optional bonus data on top of the SH estimates.
	local.cubemap_available =
		properties_cubemap.supportsCubemapLightEstimation &&
		_cubemap_load_fns   () &&
		_cubemap_pick_settings();

	if (local.cubemap_available)
		log_diagf("Light estimation available, with %dpx cubemaps (format %d).", local.cubemap_resolution, local.cubemap_format);
	else
		log_diag("Light estimation available, spherical harmonics only.");

	local.available = true;
	return xr_system_succeed;
}

///////////////////////////////////////////

void xr_ext_android_light_estimation_shutdown(void*) {
	xr_ext_light_estimation_stop();
	OPENXR_CLEAR_FN(XR_EXT_FUNCTIONS);
	OPENXR_CLEAR_FN(XR_EXT_CUBEMAP_FUNCTIONS);
	sk_free(local.cubemap_buffer);
	sk_free(local.cubemap_repack);
	local = {};
}

///////////////////////////////////////////

void xr_ext_android_light_estimation_step_begin(void*) {
	if (!local.started) return;

	// If the fine scene permission was granted after the estimator was
	// created SH-only, recreate it to pick up cubemap estimates.
	if (local.cubemap_available && !local.cubemap_started && !local.cubemap_refused &&
		permission_state(permission_type_scene_fine) == permission_state_granted) {
		xr_ext_light_estimation_stop ();
		xr_ext_light_estimation_start();
		if (!local.started) return;
	}

	XrLightEstimateGetInfoANDROID info = {(XrStructureType)XR_TYPE_LIGHT_ESTIMATE_GET_INFO_ANDROID};
	info.space = xr_app_space;
	info.time  = xr_time;

	// The cubemap is deliberately NOT chained into this per-frame poll:
	// servicing a cubemap request costs the runtime real GPU time
	// (reprojection + readback) every call, whether or not the estimate
	// changed. xr_ext_light_estimation_fetch_reflection pulls it on demand.
	XrSphericalHarmonicsANDROID  sh       = {(XrStructureType)XR_TYPE_SPHERICAL_HARMONICS_ANDROID};
	XrLightEstimateANDROID       estimate = {(XrStructureType)XR_TYPE_LIGHT_ESTIMATE_ANDROID};
	estimate.next = &sh;
	// KIND_TOTAL bakes the main light into the SH, which is what StereoKit
	// needs, since its PBR shading has no analytic light to add back. The
	// Android docs describe TOTAL/AMBIENT backwards (as of 2026-07); device
	// captures confirm TOTAL's DC term carries the full environment's energy.
	sh.kind = XR_SPHERICAL_HARMONICS_KIND_TOTAL_ANDROID;
	XrResult result = xrGetLightEstimateANDROID(local.estimator, &info, &estimate);
	if (XR_FAILED(result)) {
		log_warnf("%s: [%s]", "xrGetLightEstimateANDROID", openxr_string(result));
		return;
	}

	if (local.last_update != estimate.lastUpdatedTime) {
		local.last_update = estimate.lastUpdatedTime;
		if (sh.state == XR_LIGHT_ESTIMATE_STATE_VALID_ANDROID) {
			memcpy(local.sh_data.coefficients, sh.coefficients, sizeof(local.sh_data.coefficients));
			// ARCore's SH basis sign-flips the odd-m terms relative to
			// StereoKit's (see sh_add), so unconverted coefficients light the
			// scene rotated 180 degrees around Z, where a ceiling light reads
			// as a floor light. Negate the terms odd under (x,y) -> (-x,-y).
			// Verified on Galaxy XR against the room's real lighting.
			local.sh_data.coefficients[1] = -local.sh_data.coefficients[1];
			local.sh_data.coefficients[3] = -local.sh_data.coefficients[3];
			local.sh_data.coefficients[5] = -local.sh_data.coefficients[5];
			local.sh_data.coefficients[7] = -local.sh_data.coefficients[7];
			local.sh_updated = true;
		}
	}
}

///////////////////////////////////////////

bool xr_ext_android_light_estimation_available() {
	if (!local.available) return false;

	// 'unavailable' means the scene permission isn't in the AndroidManifest,
	// so it can never be granted. 'capable' is fine; lighting_set_mode asks.
	permission_state_ perms = permission_state(permission_type_scene);
	return perms != permission_state_unavailable && perms != permission_state_unknown;
}

///////////////////////////////////////////

bool xr_ext_light_estimation_start() {
	if (!local.available) return false;
	if (local.started)    return true;

	// Cubemap estimates show imagery of the user's surroundings, so they need
	// the fine scene permission on top of the coarse one. Without it we start
	// SH-only, and step_begin upgrades the estimator if it arrives later.
	bool use_cubemap = local.cubemap_available && !local.cubemap_refused &&
		permission_state(permission_type_scene_fine) == permission_state_granted;

	XrLightEstimatorCreateInfoANDROID        info         = {(XrStructureType)XR_TYPE_LIGHT_ESTIMATOR_CREATE_INFO_ANDROID};
	XrCubemapLightEstimatorCreateInfoANDROID cubemap_info = {(XrStructureType)XR_TYPE_CUBEMAP_LIGHT_ESTIMATOR_CREATE_INFO_ANDROID};
	if (use_cubemap) {
		cubemap_info.cubemapResolution = local.cubemap_resolution;
		cubemap_info.colorFormat       = local.cubemap_format;
		cubemap_info.reproject         = XR_TRUE;
		info.next = &cubemap_info;
	}
	XrResult result = xrCreateLightEstimatorANDROID(xr_session, &info, &local.estimator);

	// If the runtime refuses the cubemap request, fall back to SH-only
	// estimation rather than losing light estimation entirely.
	if (XR_FAILED(result) && use_cubemap) {
		log_warnf("%s: [%s], retrying without cubemap estimation", "xrCreateLightEstimatorANDROID", openxr_string(result));
		local.cubemap_refused = true;
		use_cubemap = false;
		info.next   = nullptr;
		result = xrCreateLightEstimatorANDROID(xr_session, &info, &local.estimator);
	}
	if (XR_FAILED(result)) {
		log_warnf("%s: [%s]", "xrCreateLightEstimatorANDROID", openxr_string(result));
		return false;
	}

	local.cubemap_started = use_cubemap;
	if (local.cubemap_started && local.cubemap_buffer == nullptr) {
		uint32_t px_bytes =
			local.cubemap_format == XR_CUBEMAP_LIGHTING_COLOR_FORMAT_R16G16B16A16_SFLOAT_ANDROID ? 8  :
			local.cubemap_format == XR_CUBEMAP_LIGHTING_COLOR_FORMAT_R32G32B32A32_SFLOAT_ANDROID ? 16 :
			                                                                                       12; // R32G32B32
		local.cubemap_face_bytes = local.cubemap_resolution * local.cubemap_resolution * px_bytes;
		local.cubemap_buffer     = sk_malloc_t(uint8_t, (size_t)local.cubemap_face_bytes * 6);
	}

	local.started = true;
	return true;
}

///////////////////////////////////////////

void xr_ext_light_estimation_stop() {
	if (!local.started) return;

	if (local.estimator != XR_NULL_HANDLE) {
		xrDestroyLightEstimatorANDROID(local.estimator);
		local.estimator = XR_NULL_HANDLE;
	}
	local.started         = false;
	local.cubemap_started = false;
}

///////////////////////////////////////////

bool xr_ext_light_estimation_update_sh(spherical_harmonics_t* ref_sh) {
	if (local.sh_updated) {
		local.sh_updated = false;
		*ref_sh = local.sh_data;
		return true;
	}
	return false;
}

///////////////////////////////////////////

bool xr_ext_light_estimation_cubemap_available() {
	return local.cubemap_available && !local.cubemap_refused;
}

///////////////////////////////////////////

bool xr_ext_light_estimation_reflection_info(tex_format_* out_format, int32_t* out_face_size) {
	if (!local.cubemap_started) return false;
	if (out_format)    *out_format    = local.cubemap_tex_format;
	if (out_face_size) *out_face_size = (int32_t)local.cubemap_resolution;
	return true;
}

///////////////////////////////////////////

bool xr_ext_light_estimation_fetch_reflection(tex_t ref_cubemap) {
	if (!local.cubemap_started || ref_cubemap == nullptr) return false;

	// A dedicated estimate call for the cubemap data, made only when the
	// caller has already decided the reflection needs a rebuild.
	uint32_t face_bytes = local.cubemap_face_bytes;
	XrCubemapLightingDataANDROID cubemap = {(XrStructureType)XR_TYPE_CUBEMAP_LIGHTING_DATA_ANDROID};
	// Buffers stay in the runtime's GL convention, converted below.
	cubemap.imageBufferSize   = face_bytes;
	cubemap.imageBufferRight  = local.cubemap_buffer + 0 * face_bytes;
	cubemap.imageBufferLeft   = local.cubemap_buffer + 1 * face_bytes;
	cubemap.imageBufferTop    = local.cubemap_buffer + 2 * face_bytes;
	cubemap.imageBufferBottom = local.cubemap_buffer + 3 * face_bytes;
	cubemap.imageBufferFront  = local.cubemap_buffer + 4 * face_bytes;
	cubemap.imageBufferBack   = local.cubemap_buffer + 5 * face_bytes;

	XrLightEstimateGetInfoANDROID info = {(XrStructureType)XR_TYPE_LIGHT_ESTIMATE_GET_INFO_ANDROID};
	info.space = xr_app_space;
	info.time  = xr_time;
	XrLightEstimateANDROID estimate = {(XrStructureType)XR_TYPE_LIGHT_ESTIMATE_ANDROID};
	estimate.next = &cubemap;
	XrResult result = xrGetLightEstimateANDROID(local.estimator, &info, &estimate);
	if (XR_FAILED(result)) {
		log_warnf("%s: [%s]", "xrGetLightEstimateANDROID", openxr_string(result));
		return false;
	}
	if (cubemap.state != XR_LIGHT_ESTIMATE_STATE_VALID_ANDROID) return false;

	// reproject is enabled, so rotation should be identity. If a runtime
	// returns one anyway, reflections will be angled.
	static bool warned_rotation = false;
	if (!warned_rotation && fabsf(cubemap.rotation.w) < 0.999f) {
		warned_rotation = true;
		log_warn("Light estimation cubemap arrived with a non-identity rotation, reflections may be rotated.");
	}

	int32_t res     = (int32_t)local.cubemap_resolution;
	int32_t face_px = res * res;
	void*   faces[6];

	// Convert the runtime's GL-convention faces to StereoKit's Vulkan/D3D
	// layout: buffer order right/left/top/bottom/front/back maps straight
	// onto +X/-X/+Y/-Y/+Z/-Z, with the X and Z faces mirrored horizontally
	// and the Y faces flipped vertically. Solved from a device capture, this
	// is seam consistent and matches the runtime's own SH estimate at +0.997.
	static const bool mirror_h[6] = { true, true, false, false, true, true };

	int32_t src_bytes = local.cubemap_format == XR_CUBEMAP_LIGHTING_COLOR_FORMAT_R16G16B16A16_SFLOAT_ANDROID ? 8  :
	                    local.cubemap_format == XR_CUBEMAP_LIGHTING_COLOR_FORMAT_R32G32B32A32_SFLOAT_ANDROID ? 16 : 12;
	bool    repack    = local.cubemap_format == XR_CUBEMAP_LIGHTING_COLOR_FORMAT_R32G32B32_SFLOAT_ANDROID;
	int32_t dst_bytes = repack ? 16 : src_bytes; // RGB32F -> RGBA32F

	if (local.cubemap_repack == nullptr)
		local.cubemap_repack = sk_malloc_t(uint8_t, (size_t)face_px * dst_bytes * 6);

	for (int32_t layer = 0; layer < 6; layer++) {
		const uint8_t* src = local.cubemap_buffer + (size_t)layer * local.cubemap_face_bytes;
		uint8_t*       dst = local.cubemap_repack + (size_t)layer * face_px * dst_bytes;
		for (int32_t y = 0; y < res; y++) {
			int32_t src_y = mirror_h[layer] ? y : res - 1 - y;
			for (int32_t x = 0; x < res; x++) {
				int32_t        src_x  = mirror_h[layer] ? res - 1 - x : x;
				const uint8_t* px_in  = src + ((size_t)src_y * res + src_x) * src_bytes;
				uint8_t*       px_out = dst + ((size_t)y     * res + x    ) * dst_bytes;
				if (repack) {
					((float*)px_out)[0] = ((const float*)px_in)[0];
					((float*)px_out)[1] = ((const float*)px_in)[1];
					((float*)px_out)[2] = ((const float*)px_in)[2];
					((float*)px_out)[3] = 1;
				} else {
					memcpy(px_out, px_in, src_bytes);
				}
			}
		}
		faces[layer] = dst;
	}

	// The estimate's SH describes the same environment, so setting it here
	// skips a redundant GPU SH pass on upload.
	if (ref_cubemap->light_info == nullptr)
		ref_cubemap->light_info = sk_malloc_t(spherical_harmonics_t, 1);
	*ref_cubemap->light_info = local.sh_data;

	tex_set_color_arr(ref_cubemap, res, res, faces, 6);
	return true;
}

}
