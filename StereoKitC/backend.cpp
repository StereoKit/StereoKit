#include "stereokit.h"
#include "_stereokit.h"
#include "libraries/sk_gpu.h"
#include "platforms/platform_utils.h"

namespace sk {


///////////////////////////////////////////

backend_xr_type_ backend_xr_get_type() {
	if (sk_display_mode == display_mode_mixedreality) {
#if defined(SK_XR_OPENXR)
		return backend_xr_type_openxr;
#elif defined(SK_XR_WEBXR)
		return backend_xr_type_webxr;
#else
		log_err("Unimplemented XR backend code") // <-- Haha, see what I did there? No semicolon! :D
#endif
	} else {
		if (sk_settings.disable_flatscreen_mr_sim) return backend_xr_type_none;
		else                                       return backend_xr_type_simulator;
	}
}

///////////////////////////////////////////

#if !defined(SK_XR_OPENXR)

///////////////////////////////////////////

openxr_handle_t backend_openxr_get_instance() {
	log_err("backend_openxr_ functions only work when OpenXR is the backend!");
	return 0;
}

///////////////////////////////////////////

openxr_handle_t backend_openxr_get_session() {
	log_err("backend_openxr_ functions only work when OpenXR is the backend!");
	return 0;
}

///////////////////////////////////////////

openxr_handle_t backend_openxr_get_system_id() {
	log_err("backend_openxr_ functions only work when OpenXR is the backend!");
	return 0;
}

///////////////////////////////////////////

openxr_handle_t backend_openxr_get_space() {
	log_err("backend_openxr_ functions only work when OpenXR is the backend!");
	return 0;
}

///////////////////////////////////////////

int64_t backend_openxr_get_time() {
	log_err("backend_openxr_ functions only work when OpenXR is the backend!");
	return 0;
}

///////////////////////////////////////////

void *backend_openxr_get_function(const char *function_name) {
	log_err("backend_openxr_ functions only work when OpenXR is the backend!");
	return nullptr;
}

///////////////////////////////////////////

bool32_t backend_openxr_ext_enabled(const char *extension_name) {
	log_err("backend_openxr_ functions only work when OpenXR is the backend!");
	return false;
}

///////////////////////////////////////////

void backend_openxr_ext_request(const char *extension_name) {
	if (sk_initialized) {
		log_err("backend_openxr_ext_request must be called BEFORE StereoKit initialization!");
		return;
	}
}

///////////////////////////////////////////

void backend_openxr_add_callback_pre_session_create(void (*on_pre_session_create)(void* context), void* context) {
	if (sk_initialized) {
		log_err("backend_openxr_add_callback_pre_session_create must be called BEFORE StereoKit initialization!");
		return;
	}
}

///////////////////////////////////////////

void backend_openxr_composition_layer(void *XrCompositionLayerBaseHeader, int32_t layer_size, int32_t sort_order) {
	log_err("backend_openxr_ functions only work when OpenXR is the backend!");
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
void *backend_android_get_java_vm () { log_err("backend_ platform functions only work on the correct platform!"); return nullptr; }
void *backend_android_get_activity() { log_err("backend_ platform functions only work on the correct platform!"); return nullptr; }
void *backend_android_get_jni_env () { log_err("backend_ platform functions only work on the correct platform!"); return nullptr; }
#endif

///////////////////////////////////////////

backend_graphics_ backend_graphics_get() {
#if defined(SKG_DIRECT3D11)
	return backend_graphics_d3d11;
#elif defined(SKG_OPENGL)
	return backend_graphics_none;
#endif
}

///////////////////////////////////////////

void *backend_d3d11_get_d3d_device() {
#if !defined(SKG_DIRECT3D11)
	log_err("backend_d3d11_ functions only work when D3D11 is the backend!");
	return nullptr;
#else
	void *d3d_device;
	void *d3d_context;
	render_get_device((void **)&d3d_device, (void **)&d3d_context);
	return d3d_device;
#endif
}

///////////////////////////////////////////

void *backend_d3d11_get_d3d_context() {
#if !defined(SKG_DIRECT3D11)
	log_err("backend_d3d11_ functions only work when D3D11 is the backend!");
	return nullptr;
#else
	void *d3d_device;
	void *d3d_context;
	render_get_device((void **)&d3d_device, (void **)&d3d_context);
	return d3d_context;
#endif
}

}