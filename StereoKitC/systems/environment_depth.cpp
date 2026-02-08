/* SPDX-License-Identifier: MIT */
/* The authors below grant copyright rights under the MIT license:
 * Copyright (c) 2026 Nick Klingensmith
 */

#include "environment_depth.h"

#include "../xr_backends/openxr.h"
#include "../xr_backends/extensions/ext_management.h"
#include "../xr_backends/extensions/meta_environment_depth.h"

namespace sk {

///////////////////////////////////////////

environment_depth_system_ env_depth_sys         = environment_depth_system_none;
bool32_t                  env_depth_initialized = false;

///////////////////////////////////////////

bool environment_depth_init() {
	if (env_depth_initialized) return false;

	if (xr_ext_meta_environment_depth_available())
		env_depth_sys = environment_depth_system_openxr_meta;
	else
		env_depth_sys = environment_depth_system_none;

	env_depth_initialized = true;

	switch (env_depth_sys) {
	case environment_depth_system_openxr_meta: log_diagf("Using META environment depth."); break;
	default:                                   log_diagf("No environment depth provider is supported for this runtime.");  break;
	}

	return true;
}

///////////////////////////////////////////

void environment_depth_shutdown(void*) {
	if (!env_depth_initialized) return;
	env_depth_sys         = environment_depth_system_none;
	env_depth_initialized = false;
}

///////////////////////////////////////////

void environment_depth_register() {
	xr_system_t system = {};
	system.evt_initialize = { [](void*) { return environment_depth_init() ? xr_system_succeed : xr_system_fail; } };
	system.evt_shutdown   = { environment_depth_shutdown };
	ext_management_sys_register(system);
}

///////////////////////////////////////////

bool32_t environment_depth_available() {
	switch (env_depth_sys) {
	case environment_depth_system_openxr_meta: return xr_ext_meta_environment_depth_available();
	default: return false;
	}
}

///////////////////////////////////////////

bool32_t environment_depth_running() {
	switch (env_depth_sys) {
	case environment_depth_system_openxr_meta: return xr_ext_meta_environment_depth_running();
	default: return false;
	}
}

///////////////////////////////////////////

bool32_t environment_depth_supports_hand_removal() {
	switch (env_depth_sys) {
	case environment_depth_system_openxr_meta: return xr_ext_meta_environment_depth_supports_hand_removal();
	default: return false;
	}
}

///////////////////////////////////////////

bool32_t environment_depth_start() {
	switch (env_depth_sys) {
	case environment_depth_system_openxr_meta: return xr_ext_meta_environment_depth_start();
	default: return false;
	}
}

///////////////////////////////////////////

void environment_depth_stop() {
	switch (env_depth_sys) {
	case environment_depth_system_openxr_meta: xr_ext_meta_environment_depth_stop(); return;
	default: return;
	}
}

///////////////////////////////////////////

bool32_t environment_depth_set_hand_removal(bool32_t enabled) {
	switch (env_depth_sys) {
	case environment_depth_system_openxr_meta: return xr_ext_meta_environment_depth_set_hand_removal(enabled);
	default: return false;
	}
}

///////////////////////////////////////////

bool32_t environment_depth_try_get_latest_frame(environment_depth_frame_t* out_frame) {
	if (out_frame == nullptr) return false;
	switch (env_depth_sys) {
	case environment_depth_system_openxr_meta: return xr_ext_meta_environment_depth_try_get_latest(out_frame);
	default: *out_frame = {}; return false;
	}
}

///////////////////////////////////////////

} // namespace sk