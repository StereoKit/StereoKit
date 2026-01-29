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
	bool                    available;
	XrLightEstimatorANDROID estimator;
	XrTime                  last_update;
	bool                    needs_skytex;
	sk::spherical_harmonics_t   sh_data;
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

	// Check the system properties, see if we can actually do hand
	// tracking.
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

	local.available = true;

	XrLightEstimatorCreateInfoANDROID info = {(XrStructureType)XR_TYPE_LIGHT_ESTIMATOR_CREATE_INFO_ANDROID};
	result = xrCreateLightEstimatorANDROID(xr_session, &info, &local.estimator);
	if (XR_FAILED(result)) {
		log_warnf("%s: [%s]", "xrCreateLightEstimatorANDROID", openxr_string(result));
		return xr_system_fail;
	}

	return xr_system_succeed;
}

///////////////////////////////////////////

void xr_ext_android_light_estimation_shutdown(void*) {
	xrDestroyLightEstimatorANDROID(local.estimator);
	OPENXR_CLEAR_FN(XR_EXT_FUNCTIONS);
	local = {};
}

///////////////////////////////////////////

void xr_ext_android_light_estimation_step_begin(void*) {
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

	if (local.last_update != estimate.lastUpdatedTime && estimate.state == XR_LIGHT_ESTIMATE_STATE_VALID_ANDROID) {
		local.last_update =  estimate.lastUpdatedTime;
		
		memcpy(local.sh_data.coefficients, sh.coefficients, sizeof(vec3)*9);
		render_set_skylight(local.sh_data);

		tex_t tex = tex_gen_cubemap_sh(local.sh_data, 32, 0.2f, 2);
		render_set_skytex(tex);
		tex_release(tex);

		local.needs_skytex = true;
	} else if (local.needs_skytex) {
		local.needs_skytex = false;
	}
}

}