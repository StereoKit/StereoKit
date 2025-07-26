// SPDX-License-Identifier: MIT
// The authors below grant copyright rights under the MIT license:
// Copyright (c) 2019-2023 Nick Klingensmith
// Copyright (c) 2023 Qualcomm Technologies, Inc.

#include "device.h"
#include "stereokit.h"
#include "sk_memory.h"
#include "xr_backends/openxr_view.h"

namespace sk {

device_data_t device_data = {};

///////////////////////////////////////////

void device_data_init(device_data_t* data) {
	*data = {};
}

///////////////////////////////////////////

void device_data_free(device_data_t* data) {
	sk_free(data->name);
	sk_free(data->runtime);
	sk_free(data->gpu);
	*data = {};
}

///////////////////////////////////////////

display_type_ device_display_get_type() {
	return device_data.display_type;
}

///////////////////////////////////////////

display_blend_ device_display_get_blend() {
	return device_data.display_blend;
}

///////////////////////////////////////////

bool32_t device_display_set_blend(display_blend_ blend) {
#if defined(SK_XR_OPENXR)
	if (backend_xr_get_type() == backend_xr_type_openxr) {
		return xr_set_blend(blend);
	}
#endif
	return false;
}

///////////////////////////////////////////

bool32_t device_display_valid_blend(display_blend_ blend) {
#if defined(SK_XR_OPENXR)
	if (backend_xr_get_type() == backend_xr_type_openxr) {
		return xr_blend_valid(blend);
	}
#endif
	return blend == display_blend_opaque;
}

///////////////////////////////////////////

float device_display_get_refresh_rate() {
	return device_data.display_refresh_rate;
}

///////////////////////////////////////////

int32_t device_display_get_width() {
	return device_data.display_width;
}

///////////////////////////////////////////

int32_t device_display_get_height() {
	return device_data.display_height;
}

///////////////////////////////////////////

fov_info_t device_display_get_fov() {
	return device_data.display_fov;
}

///////////////////////////////////////////

device_tracking_ device_get_tracking() {
	return device_data.tracking;
}

///////////////////////////////////////////

const char* device_get_name() {
	return device_data.name;
}

///////////////////////////////////////////

const char* device_get_runtime() {
	return device_data.runtime;
}

///////////////////////////////////////////

uint64_t device_get_runtime_version() {
	return device_data.runtime_version;
}

///////////////////////////////////////////

const char* device_get_gpu() {
	return device_data.gpu;
}

///////////////////////////////////////////

bool32_t device_has_eye_gaze() {
	return device_data.has_eye_gaze;
}

///////////////////////////////////////////

bool32_t device_has_hand_tracking() {
	return device_data.has_hand_tracking;
}

}