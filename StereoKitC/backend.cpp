#include "stereokit.h"
#include "_stereokit.h"
#include "platforms/platform.h"

#include <sk_renderer.h>

namespace sk {

const char* backend_err_wrong_backend = "Backend functions only work with the correct backend! Please check your backend info before calling.";

///////////////////////////////////////////

backend_xr_type_ backend_xr_get_type() {
	if (device_display_get_type() == display_type_stereo) {
#if defined(SK_XR_OPENXR)
		return backend_xr_type_openxr;
#elif defined(SK_XR_WEBXR)
		return backend_xr_type_webxr;
#else
		log_err("Unimplemented XR backend code") // <-- Haha, see what I did there? No semicolon! :D
#endif
	} else {
		return sk_get_settings_ref()->mode == app_mode_simulator
			? backend_xr_type_simulator
			: backend_xr_type_none;
	}
}

///////////////////////////////////////////

#if !defined(SK_XR_OPENXR)

///////////////////////////////////////////

openxr_handle_t backend_openxr_get_instance() {
	log_err(backend_err_wrong_backend);
	return 0;
}

///////////////////////////////////////////

openxr_handle_t backend_openxr_get_session() {
	log_err(backend_err_wrong_backend);
	return 0;
}

///////////////////////////////////////////

openxr_handle_t backend_openxr_get_system_id() {
	log_err(backend_err_wrong_backend);
	return 0;
}

///////////////////////////////////////////

openxr_handle_t backend_openxr_get_space() {
	log_err(backend_err_wrong_backend);
	return 0;
}

///////////////////////////////////////////

openxr_handle_t backend_openxr_get_head_space() {
	log_err(backend_err_wrong_backend);
	return 0;
}

///////////////////////////////////////////

int64_t backend_openxr_get_time() {
	log_err(backend_err_wrong_backend);
	return 0;
}

///////////////////////////////////////////

void *backend_openxr_get_function(const char *function_name) {
	log_err(backend_err_wrong_backend);
	return nullptr;
}

///////////////////////////////////////////

bool32_t backend_openxr_ext_enabled(const char *extension_name) {
	log_err(backend_err_wrong_backend);
	return false;
}

///////////////////////////////////////////

void backend_openxr_ext_request(const char *extension_name) {
	if (sk_is_initialized()) {
		log_err("backend_openxr_ext_request must be called BEFORE StereoKit initialization!");
		return;
	}
}

///////////////////////////////////////////

void backend_openxr_use_minimum_exts(bool32_t use_minimum_exts) {
	if (sk_is_initialized()) {
		log_err("backend_openxr_use_minimum_exts must be called BEFORE StereoKit initialization!");
		return;
	}
}

///////////////////////////////////////////

void backend_openxr_add_callback_pre_session_create(void (*on_pre_session_create)(void* context), void* context) {
	if (sk_is_initialized()) {
		log_err("backend_openxr_add_callback_pre_session_create must be called BEFORE StereoKit initialization!");
		return;
	}
}

///////////////////////////////////////////

void backend_openxr_add_callback_poll_event(void (*on_poll_event)(void* context, void* XrEventDataBuffer), void* context) {
	log_err(backend_err_wrong_backend);
}

///////////////////////////////////////////

void backend_openxr_remove_callback_poll_event(void (*on_poll_event)(void* context, void* XrEventDataBuffer)) {
	log_err(backend_err_wrong_backend);
}

///////////////////////////////////////////

void backend_openxr_composition_layer(void *XrCompositionLayerBaseHeader, int32_t layer_size, int32_t sort_order) {
	log_err(backend_err_wrong_backend);
}

///////////////////////////////////////////

void backend_openxr_set_hand_joint_scale(float joint_scale_factor) {
}

///////////////////////////////////////////

#endif

///////////////////////////////////////////

backend_platform_ backend_platform_get() {
#if   defined(SK_OS_ANDROID)
	return backend_platform_android;
#elif defined(SK_OS_MACOS)
	return backend_platform_macos;
#elif defined(SK_OS_LINUX)
	return backend_platform_linux;
#elif defined(SK_OS_WINDOWS)
	return backend_platform_win32;
#endif
}

///////////////////////////////////////////

#if !defined(SK_OS_ANDROID)
void *backend_android_get_java_vm () { log_err(backend_err_wrong_backend); return nullptr; }
void *backend_android_get_activity() { log_err(backend_err_wrong_backend); return nullptr; }
void *backend_android_get_jni_env () { log_err(backend_err_wrong_backend); return nullptr; }
#endif

///////////////////////////////////////////

backend_graphics_ backend_graphics_get() {
	return backend_graphics_vulkan;
}

///////////////////////////////////////////

int32_t backend_vulkan_get_frame_fence_fd() {
	return skr_renderer_frame_fence_fd();
}

///////////////////////////////////////////

void *backend_vulkan_get_instance       () { return skr_get_vk_instance       (); }
void *backend_vulkan_get_physical_device() { return skr_get_vk_physical_device(); }
void *backend_vulkan_get_device         () { return skr_get_vk_device         (); }

///////////////////////////////////////////

void *backend_vulkan_get_queue(backend_vulkan_queue_ queue) {
	switch (queue) {
	// Only the graphics queue currently has a handle available. Transfer and
	// video decode expose family indices only, until StereoKit makes real use
	// of them.
	case backend_vulkan_queue_graphics: return skr_get_vk_graphics_queue();
	default:                            return nullptr;
	}
}

///////////////////////////////////////////

uint32_t backend_vulkan_get_queue_family_index(backend_vulkan_queue_ queue) {
	switch (queue) {
	case backend_vulkan_queue_graphics:     return skr_get_vk_graphics_queue_family     ();
	case backend_vulkan_queue_transfer:     return skr_get_vk_transfer_queue_family     ();
	case backend_vulkan_queue_video_decode: return skr_get_vk_video_decode_queue_family ();
	default:                                return UINT32_MAX;
	}
}

///////////////////////////////////////////

void backend_vulkan_queue_lock(backend_vulkan_queue_ queue) {
	skr_vk_queue_lock(backend_vulkan_get_queue_family_index(queue));
}

///////////////////////////////////////////

void backend_vulkan_queue_unlock(backend_vulkan_queue_ queue) {
	skr_vk_queue_unlock(backend_vulkan_get_queue_family_index(queue));
}

///////////////////////////////////////////

void backend_vulkan_request(const backend_vulkan_request_t *request) {
	if (sk_is_initialized()) {
		log_err("backend_vulkan_request must be called BEFORE StereoKit initialization!");
		return;
	}
	skr_vk_request_t r = {};
	r.name                     = request->name;
	r.required                 = request->required != 0;
	r.instance_extensions      = request->instance_extensions;
	r.instance_extension_count = request->instance_extension_count;
	r.device_extensions        = request->device_extensions;
	r.device_extension_count   = request->device_extension_count;
	// backend_vulkan_feature_t is layout-identical to skr_vk_feature_t.
	r.features                 = (const skr_vk_feature_t*)request->features;
	r.feature_count            = request->feature_count;
	skr_vk_request(&r);
}

///////////////////////////////////////////

bool32_t backend_vulkan_request_enabled(const char *name)            { return skr_vk_request_enabled(name); }
bool32_t backend_vulkan_ext_enabled    (const char *extension_name) { return skr_vk_ext_enabled(extension_name); }
void    *backend_vulkan_get_function   (const char *function_name)  { return skr_vk_get_function(function_name); }

}
