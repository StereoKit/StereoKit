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
#elif defined(SK_OS_LINUX)
	return backend_platform_linux;
#elif defined(SK_OS_WINDOWS_UWP)
	return backend_platform_uwp;
#elif defined(SK_OS_WINDOWS)
	return backend_platform_win32;
#elif defined(SK_OS_WEB)
	return backend_platform_web;
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
// Legacy D3D11 backend functions - always return null/0 since we're Vulkan-only now

void    *backend_d3d11_get_d3d_device()           { return nullptr; }
void    *backend_d3d11_get_d3d_context()          { return nullptr; }
void    *backend_d3d11_get_deferred_d3d_context() { return nullptr; }
void    *backend_d3d11_get_deferred_mtx()         { return nullptr; }
uint32_t backend_d3d11_get_main_thread_id()       { return 0; }

///////////////////////////////////////////
// Legacy OpenGL backend functions - always return null since we're Vulkan-only now

void *backend_opengl_wgl_get_hdc()     { return nullptr; }
void *backend_opengl_wgl_get_hglrc()   { return nullptr; }
void *backend_opengl_glx_get_context() { return nullptr; }
void *backend_opengl_glx_get_display() { return nullptr; }
void *backend_opengl_glx_get_drawable(){ return nullptr; }
void *backend_opengl_egl_get_context() { return nullptr; }
void *backend_opengl_egl_get_config()  { return nullptr; }
void *backend_opengl_egl_get_display() { return nullptr; }

}
