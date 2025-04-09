/* SPDX-License-Identifier: MIT */
/* The authors below grant copyright rights under the MIT license:
 * Copyright (c) 2025 Nick Klingensmith
 * Copyright (c) 2025 Qualcomm Technologies, Inc.
 */

#include "ext_management.h"
#include "debug_utils.h"

#define XR_EXT_FUNCTIONS( X )         \
	X(xrCreateDebugUtilsMessengerEXT) \
	X(xrDestroyDebugUtilsMessengerEXT)
OPENXR_DEFINE_FN_STATIC(XR_EXT_FUNCTIONS);

typedef struct xr_debug_utils_state_t {
	XrDebugUtilsMessengerEXT messenger;
	sk::xr_system_           init_result;
} xr_debug_utils_state_t;
static xr_debug_utils_state_t local = { };

///////////////////////////////////////////

namespace sk {

///////////////////////////////////////////

void       xr_ext_debug_utils_pre_session(void*);
xr_system_ xr_ext_debug_utils_initialize (void*);
void       xr_ext_debug_utils_shutdown   (void*);

///////////////////////////////////////////

void xr_ext_debug_utils_register() {
	// Only register this extension during debug builds!
#if defined(SK_DEBUG)
	xr_system_t sys = {};
	sys.request_exts[sys.request_ext_count++] = XR_EXT_DEBUG_UTILS_EXTENSION_NAME;
	sys.func_pre_session = { xr_ext_debug_utils_pre_session };
	sys.func_initialize  = { xr_ext_debug_utils_initialize  };
	sys.func_shutdown    = { xr_ext_debug_utils_shutdown    };
	ext_management_sys_register(sys);
#endif
}

///////////////////////////////////////////

XrBool32 XRAPI_PTR openxr_debug_messenger_callback(XrDebugUtilsMessageSeverityFlagsEXT severity,
                                                   XrDebugUtilsMessageTypeFlagsEXT,
                                                   const XrDebugUtilsMessengerCallbackDataEXT *msg,
                                                   void*) {
	// Print the debug message we got! There's a bunch more info we could
	// add here too, but this is a pretty good start, and you can always
	// add a breakpoint this line!
	log_ level;
	if      (severity & XR_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT  ) level = log_error;
	else if (severity & XR_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) level = log_warning;
	else if (severity & XR_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT   ) level = log_inform;
	else                                                                 level = log_diagnostic;
	log_writef(level, "[<~mag>xr<~clr>] %s: %s", msg->functionName, msg->message);

	// Returning XR_TRUE here will force the calling function to fail
	return (XrBool32)XR_FALSE;
}

///////////////////////////////////////////

void xr_ext_debug_utils_pre_session(void*) {
	// We do this on pre-session so we start getting debug messages earlier!

	// Default to fail
	local.init_result = xr_system_fail;

	// Check if we got our extension
	if (!backend_openxr_ext_enabled(XR_EXT_DEBUG_UTILS_EXTENSION_NAME))
		return;

	// Load all extension functions
	OPENXR_LOAD_FN(XR_EXT_FUNCTIONS,);

	// Set up a really verbose debug log! Great for dev, but turn this off or
	// down for final builds. WMR doesn't produce much output here, but it
	// may be more useful for other runtimes?
	// Here's some extra information about the message types and severities:
	// https://www.khronos.org/registry/OpenXR/specs/1.0/html/xrspec.html#debug-message-categorization
	XrDebugUtilsMessengerCreateInfoEXT debug_info = { XR_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT };
	debug_info.userCallback = openxr_debug_messenger_callback;
	debug_info.messageTypes =
		XR_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT     |
		XR_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT  |
		XR_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT |
		XR_DEBUG_UTILS_MESSAGE_TYPE_CONFORMANCE_BIT_EXT;
	debug_info.messageSeverities =
		//XR_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
		XR_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT    |
		XR_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
		XR_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;

	// Start up the debug utils, and store our success/fail for the init call.
	local.init_result = XR_SUCCEEDED(xrCreateDebugUtilsMessengerEXT(xr_instance, &debug_info, &local.messenger))
		? xr_system_succeed
		: xr_system_fail;
}

///////////////////////////////////////////

xr_system_ xr_ext_debug_utils_initialize(void*) {
	// Pass along the results from the earlier pre-session event! This isn't
	// strictly necessary, but success/failure here can be used to prevent
	// addition of step/shutdown callbacks to the list. This makes behavior
	// more predicatable if we add more code here in the future.
	return local.init_result;
}

///////////////////////////////////////////

void xr_ext_debug_utils_shutdown(void*) {
	if (local.messenger) xrDestroyDebugUtilsMessengerEXT(local.messenger);
	local = {};
}

} // namespace sk