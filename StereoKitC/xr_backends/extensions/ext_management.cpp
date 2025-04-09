/* SPDX-License-Identifier: MIT */
/* The authors below grant copyright rights under the MIT license:
 * Copyright (c) 2025 Nick Klingensmith
 * Copyright (c) 2025 Qualcomm Technologies, Inc.
 */

#include "../openxr.h"

#include "ext_management.h"
#include "../../stereokit.h"
#include "../../_stereokit.h"
#include "../../libraries/array.h"
#include "../../libraries/ferr_hash.h"
#include "../../libraries/stref.h"

typedef struct ext_management_state_t {
	array_t<const char*> exts_user;
	array_t<const char*> exts_exclude;
	array_t<uint64_t>    exts_loaded;
	bool                 minimum_exts;
	bool                 exts_collected;

	array_t<sk::xr_system_t> system_list;

	array_t<sk::context_callback_t>    callbacks_pre_session_create;
	array_t<sk::context_callback_t>    callbacks_step_begin;
	array_t<sk::context_callback_t>    callbacks_step_end;
	array_t<sk::poll_event_callback_t> callbacks_poll_event;
	array_t<sk::context_callback_t>    callbacks_shutdown;
} ext_management_state_t;
static ext_management_state_t local = { };

///////////////////////////////////////////

namespace sk {

///////////////////////////////////////////

void ext_management_sys_register(xr_system_t system) {
	local.system_list.add(system);
	if (system.func_pre_session.callback)
		local.callbacks_pre_session_create.add(system.func_pre_session);

	// TODO: This registers as a USER extension currently! It should be separate!
	for (int32_t e = 0; e < system.request_ext_count; e++)
		backend_openxr_ext_request(system.request_exts[e]);
}

///////////////////////////////////////////

bool is_ext_explicitly_requested(const char* extension_name) {
	for (int32_t i = 0; i < local.exts_user.count; i++) {
		if (string_eq(local.exts_user[i], extension_name)) return true;
	}
	return false;
}

///////////////////////////////////////////

bool ext_management_get_use_min() {
	return local.minimum_exts;
}

///////////////////////////////////////////

void ext_management_mark_loaded(const char** extension_names, int32_t count) {
	for (int32_t i = 0; i < count; i++)
		local.exts_loaded.add(hash_fnv64_string(extension_names[i]));
	local.exts_collected = true;
}

///////////////////////////////////////////

void ext_management_get_exts(const char*** out_ext_names, int32_t* out_count) {
	*out_ext_names = local.exts_user.data;
	*out_count     = local.exts_user.count;
}

///////////////////////////////////////////

void ext_management_get_excludes(const char*** out_ext_names, int32_t* out_count) {
	*out_ext_names = local.exts_exclude.data;
	*out_count     = local.exts_exclude.count;
}

///////////////////////////////////////////

void ext_management_evt_pre_session_create() {
	for (int32_t i = 0; i < local.callbacks_pre_session_create.count; i++) {
		local.callbacks_pre_session_create[i].callback(local.callbacks_pre_session_create[i].context);
	}
	local.callbacks_pre_session_create.free();
}

///////////////////////////////////////////

void ext_management_evt_step_begin() {
	for (int32_t i = 0; i < local.callbacks_step_begin.count; i++) {
		context_callback_t* evt = &local.callbacks_step_begin[i];
		evt->callback(evt->context);
	}
}

///////////////////////////////////////////

void ext_management_evt_step_end() {
	for (int32_t i = 0; i < local.callbacks_step_end.count; i++) {
		context_callback_t* evt = &local.callbacks_step_end[i];
		evt->callback(evt->context);
	}
}

///////////////////////////////////////////

void ext_management_evt_poll_event(const XrEventDataBuffer* event_data) {
	for (int32_t i = 0; i < local.callbacks_poll_event.count; i++) {
		poll_event_callback_t* evt = &local.callbacks_poll_event[i];
		evt->callback(evt->context, (void*)event_data);
	}
}

///////////////////////////////////////////

bool ext_management_evt_session_ready() {
	for (int32_t i = 0; i < local.system_list.count; i++) {
		const xr_system_t* sys = &local.system_list[i];

		if (sys->func_initialize.callback) {

			xr_system_ sys_result = sys->func_initialize.callback(sys->func_initialize.context);
			if (sys_result == xr_system_fail_critical) {
				// If the system fails critically, we need to fail
				// initialization entirely!
				log_warnf("Critical failure in XR system #%d!", i);
				return false;

			} else if (sys_result == xr_system_fail) {
				// If the system fails normally, no worries, we just skip
				// adding any of its callbacks!
				continue;
			}
		}

		if (sys->func_shutdown  .callback) local.callbacks_shutdown  .add(sys->func_shutdown  );
		if (sys->func_step_begin.callback) local.callbacks_step_begin.add(sys->func_step_begin);
		if (sys->func_step_end  .callback) local.callbacks_step_end  .add(sys->func_step_end  );
	}
	return true;
}

///////////////////////////////////////////

void ext_management_cleanup() {

	// Call shutdown in reverse order. Just in case.
	for (int32_t i = local.callbacks_shutdown.count-1; i >= 0; i--) {
		const context_callback_t* sys = &local.callbacks_shutdown[i];
		sys->callback(sys->context);
	}

	local.exts_user   .free();
	local.exts_exclude.free();
	local.exts_loaded .free();

	local.system_list .free();

	local.callbacks_step_begin        .free();
	local.callbacks_step_end          .free();
	local.callbacks_shutdown          .free();
	local.callbacks_pre_session_create.free();
	local.callbacks_poll_event        .free();
	local = {};
}

///////////////////////////////////////////

bool32_t backend_openxr_ext_enabled(const char *extension_name) {
	if (backend_xr_get_type() != backend_xr_type_openxr) {
		return false;
	}
	uint64_t hash = hash_fnv64_string(extension_name);
	return local.exts_loaded.index_of(hash) >= 0 ? 1 : 0;
}

///////////////////////////////////////////

void backend_openxr_ext_request(const char *extension_name) {
	if (local.exts_collected || sk_is_initialized()) {
		log_err("backend_openxr_ext_ must be called BEFORE StereoKit initialization!");
		return;
	}
	local.exts_user.add(string_copy(extension_name));
}

///////////////////////////////////////////

void backend_openxr_ext_exclude(const char* extension_name) {
	if (local.exts_collected || sk_is_initialized()) {
		log_err("backend_openxr_ext_ must be called BEFORE StereoKit initialization!");
		return;
	}
	local.exts_exclude.add(string_copy(extension_name));
}

///////////////////////////////////////////

void backend_openxr_use_minimum_exts(bool32_t use_minimum_exts) {
	if (local.exts_collected || sk_is_initialized()) {
		log_err("backend_openxr_ext_ must be called BEFORE StereoKit initialization!");
		return;
	}
	local.minimum_exts = use_minimum_exts;
}

///////////////////////////////////////////

void backend_openxr_add_callback_pre_session_create(void (*on_pre_session_create)(void* context), void* context) {
	if (local.exts_collected || sk_is_initialized()) {
		log_err("backend_openxr_ pre_session must be called BEFORE StereoKit initialization!");
		return;
	}
	local.callbacks_pre_session_create.add({ on_pre_session_create, context });
}

///////////////////////////////////////////

void backend_openxr_add_callback_poll_event(void (*on_poll_event)(void* context, void* XrEventDataBuffer), void* context) {
	if (backend_xr_get_type() != backend_xr_type_openxr) {
		log_err("backend_openxr_ functions only work when OpenXR is the backend!");
		return;
	}
	local.callbacks_poll_event.add({ on_poll_event, context });
}

///////////////////////////////////////////

void backend_openxr_remove_callback_poll_event(void (*on_poll_event)(void* context, void* XrEventDataBuffer)) {
	if (backend_xr_get_type() != backend_xr_type_openxr) {
		log_err("backend_openxr_ functions only work when OpenXR is the backend!");
		return;
	}

	for (int32_t i = 0; i < local.callbacks_poll_event.count; i++) {
		if (local.callbacks_poll_event[i].callback == on_poll_event || on_poll_event == nullptr) {
			local.callbacks_poll_event.remove(i);
			return;
		}
	}
}

} // namespace sk