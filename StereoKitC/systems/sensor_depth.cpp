/* SPDX-License-Identifier: MIT */
/* The authors below grant copyright rights under the MIT license:
 * Copyright (c) 2026 Nick Klingensmith
 */

#include "sensor.h"
#include "sensor_depth.h"

#include "../xr_backends/openxr.h"
#include "../xr_backends/extensions/ext_management.h"
#include "../xr_backends/extensions/meta_environment_depth.h"

namespace sk {

///////////////////////////////////////////

sensor_depth_system_ sensor_depth_sys         = sensor_depth_system_none;
bool32_t             sensor_depth_initialized = false;

///////////////////////////////////////////

bool sensor_depth_init() {
	if (sensor_depth_initialized) return false;

	if (xr_ext_meta_environment_depth_available())
		sensor_depth_sys = sensor_depth_system_openxr_meta;
	else
		sensor_depth_sys = sensor_depth_system_none;

	sensor_depth_initialized = true;

	switch (sensor_depth_sys) {
	case sensor_depth_system_openxr_meta: log_diagf("Using META environment depth."); break;
	default:                              log_diagf("No depth sensor provider is supported for this runtime.");  break;
	}

	return true;
}

///////////////////////////////////////////

void sensor_depth_shutdown(void*) {
	if (!sensor_depth_initialized) return;
	sensor_readback_cleanup();
	sensor_depth_sys         = sensor_depth_system_none;
	sensor_depth_initialized = false;
}

///////////////////////////////////////////

void sensor_depth_register() {
	xr_system_t system = {};
	system.evt_initialize = { [](void*) { return sensor_depth_init() ? xr_system_succeed : xr_system_fail; } };
	system.evt_shutdown   = { sensor_depth_shutdown };
	ext_management_sys_register(system);
}

///////////////////////////////////////////

bool32_t sensor_depth_available() {
	switch (sensor_depth_sys) {
	case sensor_depth_system_openxr_meta: return xr_ext_meta_environment_depth_available();
	default: return false;
	}
}

///////////////////////////////////////////

bool32_t sensor_depth_running() {
	switch (sensor_depth_sys) {
	case sensor_depth_system_openxr_meta: return xr_ext_meta_environment_depth_running();
	default: return false;
	}
}

///////////////////////////////////////////

sensor_depth_caps_ sensor_depth_get_capabilities() {
	switch (sensor_depth_sys) {
	case sensor_depth_system_openxr_meta: return xr_ext_meta_environment_depth_get_capabilities();
	default: return sensor_depth_caps_none;
	}
}

///////////////////////////////////////////

bool32_t sensor_depth_start(sensor_depth_caps_ flags) {
	switch (sensor_depth_sys) {
	case sensor_depth_system_openxr_meta: return xr_ext_meta_environment_depth_start(flags);
	default: return false;
	}
}

///////////////////////////////////////////

void sensor_depth_stop() {
	sensor_readback_cleanup();
	switch (sensor_depth_sys) {
	case sensor_depth_system_openxr_meta: xr_ext_meta_environment_depth_stop(); return;
	default: return;
	}
}

///////////////////////////////////////////

bool32_t sensor_depth_set_capabilities(sensor_depth_caps_ flags) {
	switch (sensor_depth_sys) {
	case sensor_depth_system_openxr_meta: return xr_ext_meta_environment_depth_set_caps(flags);
	default: return false;
	}
}

///////////////////////////////////////////

tex_t sensor_depth_get_texture() {
	switch (sensor_depth_sys) {
	case sensor_depth_system_openxr_meta: return xr_ext_meta_environment_depth_get_texture();
	default: return nullptr;
	}
}

///////////////////////////////////////////

bool32_t sensor_depth_try_get_latest_frame(sensor_depth_frame_t* out_frame) {
	if (out_frame == nullptr) return false;
	switch (sensor_depth_sys) {
	case sensor_depth_system_openxr_meta: return xr_ext_meta_environment_depth_try_get_latest(out_frame);
	default: *out_frame = {}; return false;
	}
}

///////////////////////////////////////////

bool32_t sensor_depth_try_get_latest_data(sensor_depth_frame_t* out_frame, void* out_data, size_t* out_data_size, int32_t view_index) {
	if (out_frame == nullptr) return false;
	if (!sensor_depth_running()) {
		*out_frame = {};
		if (out_data_size) *out_data_size = 0;
		return false;
	}

	// Enable the readback pipeline
	sensor_readback_request();

	sensor_depth_frame_t frame = {};
	if (!sensor_readback_get(&frame, sizeof(frame), out_data, out_data_size, 2, view_index))
		return false;

	*out_frame = frame;
	return true;
}

///////////////////////////////////////////

} // namespace sk