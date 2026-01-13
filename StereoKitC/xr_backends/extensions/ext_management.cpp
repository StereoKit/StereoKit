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
#include "../../libraries/stref.h"
#include "../../libraries/profiler.h"

typedef struct ext_management_state_t {
	array_t<const char*>     exts_user;
	array_t<const char*>     exts_sk;
	array_t<char*>           exts_exclude_m; // these _m lists own allocated strings
	array_t<char*>           exts_all_m;
	array_t<sk::id_hash_t>   exts_loaded;
	array_t<char*>           exts_available_m; // all extensions available from OpenXR
	array_t<sk::id_hash_t>   exts_available_hashes;
	bool                     minimum_exts;
	bool                     exts_enumerated;
	bool                     exts_collected;

	array_t<sk::xr_system_t> system_list;

	array_t<sk::create_info_callback_t> callbacks_pre_instance_create;
	array_t<sk::create_info_callback_t> callbacks_pre_session_create;
	array_t<sk::create_info_callback_t> callbacks_begin_session;
	array_t<sk::context_callback_t>     callbacks_step_begin;
	array_t<sk::context_callback_t>     callbacks_step_end;
	array_t<sk::poll_event_callback_t>  callbacks_poll_event;
	array_t<sk::context_callback_t>     callbacks_shutdown;
	array_t<sk::profile_callback_t>     callbacks_profile;
} ext_management_state_t;
static ext_management_state_t local = { };

///////////////////////////////////////////

namespace sk {

///////////////////////////////////////////

int32_t in_list(array_t<char*> list, const char* str) {
	for (int32_t i = 0; i < list.count; i++)
		if (string_eq(list[i], str)) return i;
	return -1;
}

///////////////////////////////////////////

int32_t in_list(array_t<const char*> list, const char* str) {
	for (int32_t i = 0; i < list.count; i++)
		if (string_eq(list[i], str)) return i;
	return -1;
}

///////////////////////////////////////////

void ext_management_sys_register(xr_system_t system) {
	// Skip non-required systems if we're told to use the minimum number of
	// extensions.
	if (local.minimum_exts && system.required == false) {
		// If the user has manually requested all extensions this system relies
		// on, then we can use it. Or if it uses no extensions.
		for (int32_t e = 0; e < system.request_ext_count; e++) {
			bool found = false;
			for (int32_t u = 0; u < local.exts_user.count; u++) {
				if (string_eq(system.request_exts[e], local.exts_user[u])) {
					found = true;
					break;
				}
			}
			if (!found) return;
		}
	}

	local.system_list.add(system);

	// These callback hooks are _not_ dependant on a successful initialization callback
	if (system.evt_pre_instance .callback) local.callbacks_pre_instance_create.add(system.evt_pre_instance );
	if (system.evt_pre_session  .callback) local.callbacks_pre_session_create .add(system.evt_pre_session  );
	if (system.evt_begin_session.callback) local.callbacks_begin_session      .add(system.evt_begin_session);

	for (int32_t e = 0; e < system.request_ext_count; e++)
		ext_management_request_ext(system.request_exts[e]);

	// If we're using minimum exts, don't request optional extensions. We can
	// still reach this code if all required extensions were in the user requested
	// list.
	if (!local.minimum_exts) {
		for (int32_t e = 0; e < system.request_opt_ext_count; e++)
			ext_management_request_ext(system.request_opt_exts[e]);
	}
}

///////////////////////////////////////////

bool ext_management_is_user_requested(const char* extension_name) {
	for (int32_t i = 0; i < local.exts_user.count; i++) {
		if (string_eq(local.exts_user[i], extension_name)) return true;
	}
	return false;
}

///////////////////////////////////////////

void ext_management_request_ext(const char* extension_name) {
	if (local.exts_collected || sk_is_initialized()) {
		log_err("Must be called BEFORE StereoKit initialization!");
		return;
	}
	char* copy = string_copy(extension_name);
	local.exts_sk   .add(copy);
	local.exts_all_m.add(copy);
}

///////////////////////////////////////////

bool ext_management_enumerate_available() {
	if (local.exts_enumerated) return true;

	// Enumerate the list of extensions available on the system
	uint32_t ext_count = 0;
	if (XR_FAILED(xrEnumerateInstanceExtensionProperties(nullptr, 0, &ext_count, nullptr)))
		return false;
	XrExtensionProperties* exts = sk_malloc_t(XrExtensionProperties, ext_count);
	for (uint32_t i = 0; i < ext_count; i++) exts[i] = { XR_TYPE_EXTENSION_PROPERTIES };
	xrEnumerateInstanceExtensionProperties(nullptr, ext_count, &ext_count, exts);

	qsort(exts, ext_count, sizeof(XrExtensionProperties), [](const void* a, const void* b) {
		return strcmp(((XrExtensionProperties*)a)->extensionName, ((XrExtensionProperties*)b)->extensionName); });

	// Store all available extensions
	for (uint32_t i = 0; i < ext_count; i++) {
		local.exts_available_m     .add(string_copy(exts[i].extensionName));
		local.exts_available_hashes.add(hash_string(exts[i].extensionName));
	}
	local.exts_enumerated = true;

	sk_free(exts);
	return true;
}

///////////////////////////////////////////

bool ext_management_ext_available(const char* extension_name) {
	return local.exts_available_hashes.index_of(hash_string(extension_name)) >= 0;
}

///////////////////////////////////////////

bool ext_management_select_exts(bool minimum_exts, array_t<char*>* ref_all_available_exts, array_t<const char*>* ref_request_exts) {
	// Ensure available extensions have been enumerated
	if (!local.exts_enumerated && !ext_management_enumerate_available())
		return false;

	// See which of the available extensions we want to use
	for (int32_t i = 0; i < local.exts_available_m.count; i++) {
		const char* ext_name = local.exts_available_m[i];

		int32_t idx = -1;
		idx = in_list(local.exts_exclude_m, ext_name); if (idx >= 0) { ref_all_available_exts->add(string_copy(ext_name)); continue; }

		const char* str = nullptr;
		idx = in_list(local.exts_user, ext_name); if (idx >= 0) { str = local.exts_user[idx];}
		idx = in_list(local.exts_sk,   ext_name); if (idx >= 0) { str = local.exts_sk  [idx];}

		if (str) {
			ref_request_exts->add(str);
			local.exts_loaded.add(hash_string(str));
		} else {
			ref_all_available_exts->add(string_copy(ext_name));
		}
	}
	local.exts_collected = true;

	return true;
}

///////////////////////////////////////////

bool ext_management_get_use_min() {
	return local.minimum_exts;
}

///////////////////////////////////////////

void ext_management_get_exts(const char*** out_ext_names, int32_t* out_count) {
	*out_ext_names = (const char**)local.exts_all_m.data;
	*out_count     =               local.exts_all_m.count;
}

///////////////////////////////////////////

void ext_management_get_excludes(const char*** out_ext_names, int32_t* out_count) {
	*out_ext_names = (const char**)local.exts_exclude_m.data;
	*out_count     =               local.exts_exclude_m.count;
}

///////////////////////////////////////////

bool ext_management_evt_pre_instance_create(XrInstanceCreateInfo* ref_instance_info) {
	for (int32_t i = 0; i < local.callbacks_pre_instance_create.count; i++) {
		create_info_callback_t* evt = &local.callbacks_pre_instance_create[i];
		if (evt->callback(evt->context, (XrBaseHeader*)ref_instance_info) == xr_system_fail_critical)
			return false;
	}
	local.callbacks_pre_instance_create.free();
	return true;
}

///////////////////////////////////////////

bool ext_management_evt_pre_session_create(XrSessionCreateInfo* ref_session_info) {
	for (int32_t i = 0; i < local.callbacks_pre_session_create.count; i++) {
		create_info_callback_t* evt = &local.callbacks_pre_session_create[i];
		if (evt->callback(evt->context, (XrBaseHeader*)ref_session_info) == xr_system_fail_critical)
			return false;
	}
	local.callbacks_pre_session_create.free();
	return true;
}

///////////////////////////////////////////

void ext_management_evt_step_begin() {
	profiler_zone();

	for (int32_t i = 0; i < local.callbacks_step_begin.count; i++) {
		context_callback_t* evt = &local.callbacks_step_begin[i];
		evt->callback(evt->context);
	}
}

///////////////////////////////////////////

void ext_management_evt_step_end() {
	profiler_zone();

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

void ext_management_evt_profile_suggest(xr_interaction_profile_t* ref_profile) {
	for (int32_t i = 0; i < local.callbacks_profile.count; i++) {
		profile_callback_t* evt = &local.callbacks_profile[i];
		evt->callback(evt->context, ref_profile);
	}
}

///////////////////////////////////////////

bool ext_management_evt_session_ready() {
	for (int32_t i = 0; i < local.system_list.count; i++) {
		const xr_system_t* sys = &local.system_list[i];

		if (sys->evt_initialize.callback) {

			xr_system_ sys_result = sys->evt_initialize.callback(sys->evt_initialize.context);
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

		if (sys->evt_shutdown  .callback) local.callbacks_shutdown  .add(sys->evt_shutdown  );
		if (sys->evt_step_begin.callback) local.callbacks_step_begin.add(sys->evt_step_begin);
		if (sys->evt_step_end  .callback) local.callbacks_step_end  .add(sys->evt_step_end  );
		if (sys->evt_profile   .callback) local.callbacks_profile   .add(sys->evt_profile   );
		if (sys->evt_poll      .callback) local.callbacks_poll_event.add(sys->evt_poll      );
	}
	return true;
}

///////////////////////////////////////////

void ext_management_evt_session_begin(XrSessionBeginInfo* ref_session_info) {
	for (int32_t i = 0; i < local.callbacks_begin_session.count; i++) {
		create_info_callback_t* evt = &local.callbacks_begin_session[i];
		evt->callback(evt->context, (XrBaseHeader*)ref_session_info);
	}
}

///////////////////////////////////////////

void ext_management_cleanup() {

	// Call shutdown in reverse order. Just in case.
	for (int32_t i = local.callbacks_shutdown.count-1; i >= 0; i--) {
		const context_callback_t* sys = &local.callbacks_shutdown[i];
		sys->callback(sys->context);
	}

	// Some of these lists are in charge of allocated memory
	for (int32_t i =0; i<local.exts_all_m      .count; i++) sk_free(local.exts_all_m      [i]);
	for (int32_t i =0; i<local.exts_exclude_m  .count; i++) sk_free(local.exts_exclude_m  [i]);
	for (int32_t i =0; i<local.exts_available_m.count; i++) sk_free(local.exts_available_m[i]);
	local.exts_all_m           .free();
	local.exts_exclude_m       .free();
	local.exts_available_m     .free();
	local.exts_available_hashes.free();

	local.exts_sk       .free();
	local.exts_user     .free();
	local.exts_loaded   .free();
	local.system_list   .free();

	local.callbacks_step_begin         .free();
	local.callbacks_step_end           .free();
	local.callbacks_shutdown           .free();
	local.callbacks_pre_session_create .free();
	local.callbacks_pre_instance_create.free();
	local.callbacks_poll_event         .free();
	local.callbacks_profile            .free();
	local = {};
}

///////////////////////////////////////////

bool32_t backend_openxr_ext_enabled(const char *extension_name) {
	// Check xr_instance instead of backend_xr_get_type so this works during
	// early init before device_display_get_type() is set.
	if (xr_instance == XR_NULL_HANDLE) {
		return false;
	}
	return local.exts_loaded.index_of(hash_string(extension_name)) >= 0 ? 1 : 0;
}

///////////////////////////////////////////

void backend_openxr_ext_request(const char *extension_name) {
	if (local.exts_collected || sk_is_initialized()) {
		log_err("backend_openxr_ext_ must be called BEFORE StereoKit initialization!");
		return;
	}
	char* copy = string_copy(extension_name);
	local.exts_user .add(copy);
	local.exts_all_m.add(copy);
}

///////////////////////////////////////////

void backend_openxr_ext_exclude(const char* extension_name) {
	if (local.exts_collected || sk_is_initialized()) {
		log_err("backend_openxr_ext_ must be called BEFORE StereoKit initialization!");
		return;
	}
	local.exts_exclude_m.add(string_copy(extension_name));
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

// Adapter to convert the public API's callback into our newer internal
// callback.
// TODO: update the public API to use the newer format callback!
typedef struct pre_session_create_callback_data_t{
	void (*callback)(void* context);
	void*  context;
} pre_session_create_callback_data_t;
xr_system_ invoke_pre_session_create_callback(void* context, XrBaseHeader*) {
	pre_session_create_callback_data_t* callback_data = (pre_session_create_callback_data_t*)context;
	if (callback_data->callback) {
		callback_data->callback(callback_data->context);
	}
	sk_free(context);
	return xr_system_succeed;
}

void backend_openxr_add_callback_pre_session_create(void (*on_pre_session_create)(void* context), void* context) {
	if (local.exts_collected || sk_is_initialized()) {
		log_err("backend_openxr_ pre_session must be called BEFORE StereoKit initialization!");
		return;
	}

	pre_session_create_callback_data_t* callback_data = sk_malloc_t(pre_session_create_callback_data_t, 1);
	callback_data->callback = on_pre_session_create;
	callback_data->context  = context;
	local.callbacks_pre_session_create.add({ invoke_pre_session_create_callback, callback_data });
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