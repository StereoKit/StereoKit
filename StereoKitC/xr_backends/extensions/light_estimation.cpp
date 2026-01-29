#include "../../stereokit.h"
#include "../openxr.h"
#include "ext_management.h"
#include "light_estimation.h"

#include "xr_android_light_estimation.h"

#define XR_EXT_FUNCTIONS( X )  \
	X(xrCreateLightEstimatorANDROID)  \
	X(xrDestroyLightEstimatorANDROID) \
	X(xrGetLightEstimateANDROID)
OPENXR_DEFINE_FN_STATIC(XR_EXT_FUNCTIONS);

typedef struct xr_light_estimation_state_t {
	bool                      available;
	bool                      started;
	XrLightEstimatorANDROID   estimator;
	XrTime                    last_update;
	bool                      sh_updated;
	sk::spherical_harmonics_t sh_data;
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
	sys.request_exts[sys.request_ext_count++] = XR_ANDROID_LIGHT_ESTIMATION_EXTENSION_NAME;
	sys.evt_initialize = { xr_ext_android_light_estimation_initialize };
	sys.evt_shutdown   = { xr_ext_android_light_estimation_shutdown };
	sys.evt_step_begin = { xr_ext_android_light_estimation_step_begin };
	ext_management_sys_register(sys);
}

///////////////////////////////////////////

xr_system_ xr_ext_android_light_estimation_initialize(void*) {
	if (!backend_openxr_ext_enabled(XR_ANDROID_LIGHT_ESTIMATION_EXTENSION_NAME))
		return xr_system_fail;

	OPENXR_LOAD_FN_RETURN(XR_EXT_FUNCTIONS, xr_system_fail);

	// Check the system properties, see if we can actually do light estimation
	XrSystemProperties                       properties       = { XR_TYPE_SYSTEM_PROPERTIES };
	XrSystemLightEstimationPropertiesANDROID properties_light = { (XrStructureType)XR_TYPE_SYSTEM_LIGHT_ESTIMATION_PROPERTIES_ANDROID };
	properties.next = &properties_light;
	XrResult result = xrGetSystemProperties(xr_instance, xr_system_id, &properties);
	if (XR_FAILED(result)) {
		log_infof("%s: [%s]", "xrGetSystemProperties", openxr_string(result));
		return xr_system_fail;
	}
	if (properties_light.supportsLightEstimation == false)
		return xr_system_fail;

    // Check if permissions will ever allow this
    permission_state_ perms = permission_state(permission_type_scene);
    if (perms == permission_state_unavailable || perms == permission_state_unknown)
        return xr_system_fail;

	local.available = true;
	return xr_system_succeed;
}

///////////////////////////////////////////

void xr_ext_android_light_estimation_shutdown(void*) {
	xr_ext_light_estimation_stop();
	OPENXR_CLEAR_FN(XR_EXT_FUNCTIONS);
	local = {};
}

///////////////////////////////////////////

void xr_ext_android_light_estimation_step_begin(void*) {
	if (!local.started) return;

	XrLightEstimateGetInfoANDROID info = {(XrStructureType)XR_TYPE_LIGHT_ESTIMATE_GET_INFO_ANDROID};
	info.space = xr_app_space;
	info.time  = xr_time;

	XrSphericalHarmonicsANDROID sh       = {(XrStructureType)XR_TYPE_SPHERICAL_HARMONICS_ANDROID};
	XrLightEstimateANDROID      estimate = {(XrStructureType)XR_TYPE_LIGHT_ESTIMATE_ANDROID};
	estimate.next = &sh;
	sh.kind = XR_SPHERICAL_HARMONICS_KIND_TOTAL_ANDROID;
	XrResult result = xrGetLightEstimateANDROID(local.estimator, &info, &estimate);
	if (XR_FAILED(result)) {
		log_warnf("%s: [%s]", "xrGetLightEstimateANDROID", openxr_string(result));
		return;
	}

	if (local.last_update != estimate.lastUpdatedTime && sh.state == XR_LIGHT_ESTIMATE_STATE_VALID_ANDROID) {
		local.last_update  = estimate.lastUpdatedTime;
		memcpy(local.sh_data.coefficients, sh.coefficients, sizeof(local.sh_data.coefficients));
		local.sh_updated = true;
	}
}

///////////////////////////////////////////

bool xr_ext_android_light_estimation_available() {
	return local.available;
}

///////////////////////////////////////////

bool xr_ext_light_estimation_start() {
	if (!local.available) return false;
	if (local.started)    return true;

	XrLightEstimatorCreateInfoANDROID info = {(XrStructureType)XR_TYPE_LIGHT_ESTIMATOR_CREATE_INFO_ANDROID};
	XrResult result = xrCreateLightEstimatorANDROID(xr_session, &info, &local.estimator);
	if (XR_FAILED(result)) {
		log_warnf("%s: [%s]", "xrCreateLightEstimatorANDROID", openxr_string(result));
		return false;
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
	local.started = false;
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

bool xr_ext_light_estimation_update_reflection(tex_t ref_cubemap) {
	// The extensions don't support this yet, but a cubemap could be generated
	// from the SH data.
	return false;
}

}