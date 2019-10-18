#pragma comment(lib,"openxr_loader.lib")

#include "openxr.h"

#include "../../stereokit.h"
#include "../../_stereokit.h"
#include "../../systems/d3d.h"
#include "../../systems/render.h"
#include "../../systems/input.h"
#include "../../systems/input_hand.h"
#include "../../asset_types/texture.h"

#define XR_USE_PLATFORM_WIN32
#define XR_USE_GRAPHICS_API_D3D11
#include <openxr/openxr.h>
#include <openxr/openxr_platform.h>

#include <string.h>
#include <stdlib.h>
#include <vector>

using namespace std;

namespace sk {

///////////////////////////////////////////

struct swapchain_t {
	XrSwapchain handle;
	int32_t     width;
	int32_t     height;
	vector<XrSwapchainImageD3D11KHR> surface_images;
	vector<tex2d_t>                  surface_data;
};
struct xr_input_t {
	XrActionSet action_set;
	XrAction    poseAction;
	XrAction    selectAction;
	XrAction    gripAction;
	XrPath   handSubactionPath[2];
	XrSpace  handSpace[2];
	XrPosef  handPose[2];
	XrBool32 renderHand[2];
	XrBool32 handSelect[2];
	int      pointer_ids[3];
};

///////////////////////////////////////////

XrFormFactor            app_config_form = XR_FORM_FACTOR_HEAD_MOUNTED_DISPLAY;
XrViewConfigurationType app_config_view = XR_VIEW_CONFIGURATION_TYPE_PRIMARY_STEREO;

XrInstance     xr_instance      = {};
XrSession      xr_session       = {};
XrSessionState xr_session_state = XR_SESSION_STATE_UNKNOWN;
bool           xr_running         = false;
XrSpace        xr_app_space     = {};
XrSystemId     xr_system_id     = XR_NULL_SYSTEM_ID;
XrEnvironmentBlendMode xr_blend;
xr_input_t     xr_input = {};
XrTime         xr_time;

vector<XrView>                  xr_views;
vector<XrViewConfigurationView> xr_config_views;
vector<swapchain_t>             xr_swapchains;

///////////////////////////////////////////

bool openxr_render_layer(XrTime predictedTime, vector<XrCompositionLayerProjectionView> &projectionViews, XrCompositionLayerProjection &layer);
void openxr_pose_to_pointer(XrPosef &pose, pointer_t *pointer);

///////////////////////////////////////////

inline bool openxr_loc_valid(XrSpaceLocation &loc) {
	return 
		(loc.locationFlags & XR_SPACE_LOCATION_POSITION_VALID_BIT   ) != 0 &&
		(loc.locationFlags & XR_SPACE_LOCATION_ORIENTATION_VALID_BIT) != 0;
}

///////////////////////////////////////////

bool openxr_init(const char *app_name) {
	const char          *extensions[] = { XR_KHR_D3D11_ENABLE_EXTENSION_NAME };
	XrInstanceCreateInfo createInfo   = { XR_TYPE_INSTANCE_CREATE_INFO };
	createInfo.enabledExtensionCount = _countof(extensions);
	createInfo.enabledExtensionNames = extensions;
	createInfo.applicationInfo.applicationVersion = 1;
	createInfo.applicationInfo.engineVersion      = SK_VERSION_ID;
	createInfo.applicationInfo.apiVersion         = XR_CURRENT_API_VERSION;
	strcpy_s(createInfo.applicationInfo.applicationName, app_name);
	strcpy_s(createInfo.applicationInfo.engineName, "StereoKit");
	xrCreateInstance(&createInfo, &xr_instance);

	// Check if OpenXR is on this system, if this is null here, the user needs to install an
	// OpenXR runtime and ensure it's active!
	if (xr_instance == XR_NULL_HANDLE) {
		log_info("Couldn't create OpenXR instance, is OpenXR installed and set as the active runtime?");
		return false;
	}

	// Request a form factor from the device (HMD, Handheld, etc.)
	XrSystemGetInfo systemInfo = { XR_TYPE_SYSTEM_GET_INFO };
	systemInfo.formFactor = app_config_form;
	xrGetSystem(xr_instance, &systemInfo, &xr_system_id);

	// OpenXR wants to ensure apps are using the correct LUID, so this must be called before xrCreateSession
	// TODO: Figure out how to make sure we're using the correct LUID, and get it to the d3d device before initialization >.<
	// see here for reference: https://github.com/microsoft/OpenXR-SDK-VisualStudio/blob/fce13c538839a7c1f185595d6490e8d227741356/samples/BasicXrApp/DxUtility.cpp#L22
	XrGraphicsRequirementsD3D11KHR requirement = { XR_TYPE_GRAPHICS_REQUIREMENTS_D3D11_KHR };
	xrGetD3D11GraphicsRequirementsKHR(xr_instance, xr_system_id, &requirement);

	// Check what blend mode is valid for this device (opaque vs transparent displays)
	// We'll just take the first one available!
	uint32_t                       blend_count = 0;
	vector<XrEnvironmentBlendMode> blend_modes;
	xrEnumerateEnvironmentBlendModes(xr_instance, xr_system_id, app_config_view, 0, &blend_count, nullptr);
	blend_modes.resize(blend_count);
	xrEnumerateEnvironmentBlendModes(xr_instance, xr_system_id, app_config_view, blend_count, &blend_count, blend_modes.data());
	for (size_t i = 0; i < blend_count; i++) {
		if (blend_modes[i] == XR_ENVIRONMENT_BLEND_MODE_ADDITIVE || 
			blend_modes[i] == XR_ENVIRONMENT_BLEND_MODE_OPAQUE || 
			blend_modes[i] == XR_ENVIRONMENT_BLEND_MODE_ALPHA_BLEND) {
			xr_blend = blend_modes[i];
			break;
		}
	}

	// register dispay type with the system
	switch (xr_blend) {
	case XR_ENVIRONMENT_BLEND_MODE_OPAQUE: {
		sk_info.display_type = display_opaque;
	}break;
	case XR_ENVIRONMENT_BLEND_MODE_ADDITIVE: {
		sk_info.display_type = display_additive;
	}break;
	case XR_ENVIRONMENT_BLEND_MODE_ALPHA_BLEND: {
		sk_info.display_type = display_passthrough;
	}break;
	}

	// A session represents this application's desire to display things! This is where we hook up our graphics API.
	// This does not start the session, for that, you'll need a call to xrBeginSession, which we do in openxr_poll_events
	XrGraphicsBindingD3D11KHR d3d_binding = { XR_TYPE_GRAPHICS_BINDING_D3D11_KHR };
	d3d_binding.device = d3d_device;
	XrSessionCreateInfo sessionInfo = { XR_TYPE_SESSION_CREATE_INFO };
	sessionInfo.next     = &d3d_binding;
	sessionInfo.systemId = xr_system_id;
	xrCreateSession(xr_instance, &sessionInfo, &xr_session);

	// Unable to start a session, may not have an MR device attached or ready
	if (xr_session == XR_NULL_HANDLE) {
		log_info("Couldn't create an OpenXR session, no MR device attached/ready?");
		return false;
	}

	// OpenXR uses a couple different types of reference frames for positioning content, we need to choose one for
	// displaying our content! STAGE would be relative to the center of your guardian system's bounds, and LOCAL
	// would be relative to your device's starting location. HoloLens doesn't have a STAGE, so we'll use LOCAL.
	XrReferenceSpaceCreateInfo ref_space = { XR_TYPE_REFERENCE_SPACE_CREATE_INFO };
	ref_space.poseInReferenceSpace = { {0,0,0,1}, {0,0,0} };
	ref_space.referenceSpaceType   = XR_REFERENCE_SPACE_TYPE_LOCAL;
	xrCreateReferenceSpace(xr_session, &ref_space, &xr_app_space);

	// Now we need to find all the viewpoints we need to take care of! For a stereo headset, this should be 2.
	// Similarly, for an AR phone, we'll need 1, and a VR cave could have 6, or even 12!
	uint32_t view_count = 0;
	xrEnumerateViewConfigurationViews(xr_instance, xr_system_id, app_config_view, 0, &view_count, nullptr);
	xr_config_views.resize(view_count, { XR_TYPE_VIEW_CONFIGURATION_VIEW });
	xr_views       .resize(view_count, { XR_TYPE_VIEW });
	xrEnumerateViewConfigurationViews(xr_instance, xr_system_id, app_config_view, view_count, &view_count, xr_config_views.data());
	for (uint32_t i = 0; i < view_count; i++) {
		// Create a swapchain for this viewpoint! A swapchain is a set of texture buffers used for displaying to screen,
		// typically this is a backbuffer and a front buffer, one for rendering data to, and one for displaying on-screen.
		XrViewConfigurationView &view           = xr_config_views[i];
		XrSwapchainCreateInfo    swapchain_info = { XR_TYPE_SWAPCHAIN_CREATE_INFO };
		XrSwapchain              handle;
		swapchain_info.arraySize   = 1;
		swapchain_info.mipCount    = 1;
		swapchain_info.faceCount   = 1;
		swapchain_info.format      = DXGI_FORMAT_R8G8B8A8_UNORM;
		swapchain_info.width       = view.recommendedImageRectWidth;
		swapchain_info.height      = view.recommendedImageRectHeight;
		swapchain_info.sampleCount = view.recommendedSwapchainSampleCount;
		swapchain_info.usageFlags  = XR_SWAPCHAIN_USAGE_SAMPLED_BIT | XR_SWAPCHAIN_USAGE_COLOR_ATTACHMENT_BIT;
		xrCreateSwapchain(xr_session, &swapchain_info, &handle);

		// Find out how many textures were generated for the swapchain
		uint32_t surface_count = 0;
		xrEnumerateSwapchainImages(handle, 0, &surface_count, nullptr);

		// We'll want to track our own information about the swapchain, so we can draw stuff onto it! We'll also create
		// a depth buffer for each generated texture here as well with make_surfacedata.
		swapchain_t swapchain = {};
		swapchain.width  = swapchain_info.width;
		swapchain.height = swapchain_info.height;
		swapchain.handle = handle;
		swapchain.surface_images.resize(surface_count, { XR_TYPE_SWAPCHAIN_IMAGE_D3D11_KHR } );
		swapchain.surface_data  .resize(surface_count, {});
		xrEnumerateSwapchainImages(swapchain.handle, surface_count, &surface_count, (XrSwapchainImageBaseHeader*)swapchain.surface_images.data());
		for (uint32_t s = 0; s < surface_count; s++) {
			char name[64];
			sprintf_s(name, 64, "stereokit/system/rendertarget_%d_%d", i, s);
			swapchain.surface_data[s] = tex2d_create(tex_type_rendertarget, tex_format_rgba32);
			tex2d_set_id     (swapchain.surface_data[s], name);
			tex2d_setsurface (swapchain.surface_data[s], swapchain.surface_images[s].texture);
			tex2d_add_zbuffer(swapchain.surface_data[s]);
		}
		xr_swapchains.push_back(swapchain);
	}

	openxr_make_actions();

	return true;
}

///////////////////////////////////////////

void openxr_shutdown() {
	// We used a graphics API to initialize the swapchain data, so we'll
	// give it a chance to release anythig here!
	for (size_t i = 0; i < xr_swapchains.size(); i++) {
	for (size_t s = 0; s < xr_swapchains[i].surface_data.size(); s++) {
		tex2d_release(xr_swapchains[i].surface_data[s]);
	} }
	xr_swapchains.clear();
}

///////////////////////////////////////////

void openxr_step_begin() {
	openxr_poll_events();
	if (xr_running)
		openxr_poll_actions();
}

///////////////////////////////////////////

void openxr_step_end() {
	if (xr_running)
		openxr_render_frame();
}

///////////////////////////////////////////

void openxr_poll_events() {
	// This object is pretty large, making it static so we only need to create it once
	static XrEventDataBuffer event_buffer;
	// Only the data header needs cleared out each time
	event_buffer.type = XR_TYPE_EVENT_DATA_BUFFER;
	event_buffer.next = nullptr;

	while (xrPollEvent(xr_instance, &event_buffer) == XR_SUCCESS) {
		switch (event_buffer.type) {
		case XR_TYPE_EVENT_DATA_SESSION_STATE_CHANGED: {
			XrEventDataSessionStateChanged *changed = (XrEventDataSessionStateChanged*)&event_buffer;
			xr_session_state = changed->state;
			sk_focused       = xr_session_state == XR_SESSION_STATE_VISIBLE || xr_session_state == XR_SESSION_STATE_FOCUSED;

			// Session state change is where we can begin and end sessions, as well as find quit messages!
			switch (xr_session_state) {
			case XR_SESSION_STATE_READY: {
				XrSessionBeginInfo begin_info = { XR_TYPE_SESSION_BEGIN_INFO };
				begin_info.primaryViewConfigurationType = app_config_view;
				xrBeginSession(xr_session, &begin_info);
				xr_running = true;
			} break;
			case XR_SESSION_STATE_STOPPING:     xrEndSession(xr_session); xr_running = false; break;
			case XR_SESSION_STATE_EXITING:      sk_run = false;              break;
			case XR_SESSION_STATE_LOSS_PENDING: sk_run = false;              break;
			}
		} break;
		case XR_TYPE_EVENT_DATA_INSTANCE_LOSS_PENDING: sk_run = false; return;
		}
	}
}

///////////////////////////////////////////

void openxr_render_frame() {
	// Block until the previous frame is finished displaying, and is ready for another one.
	// Also returns a prediction of when the next frame will be displayed, for use with predicting
	// locations of controllers, viewpoints, etc.
	XrFrameState frame_state = { XR_TYPE_FRAME_STATE };
	xrWaitFrame (xr_session, nullptr, &frame_state);
	// Must be called before any rendering is done! This can return some interesting flags, like 
	// XR_SESSION_VISIBILITY_UNAVAILABLE, which means we could skip rendering this frame and call
	// xrEndFrame right away.
	xrBeginFrame(xr_session, nullptr);

	// Timing also needs some work, may be best as some sort of anchor system
	xr_time = frame_state.predictedDisplayTime + frame_state.predictedDisplayPeriod;

	// Execute any code that's dependant on the predicted time, such as updating the location of
	// controller models.
	//openxr_poll_predicted(frame_state.predictedDisplayTime);

	// If the session is active, lets render our layer in the compositor!
	XrCompositionLayerBaseHeader            *layer      = nullptr;
	XrCompositionLayerProjection             layer_proj = { XR_TYPE_COMPOSITION_LAYER_PROJECTION };
	vector<XrCompositionLayerProjectionView> views;
	bool session_active = xr_session_state == XR_SESSION_STATE_VISIBLE || xr_session_state == XR_SESSION_STATE_FOCUSED;
	if (session_active && openxr_render_layer(frame_state.predictedDisplayTime, views, layer_proj)) {
		layer = (XrCompositionLayerBaseHeader*)&layer_proj;
	}

	// We're finished with rendering our layer, so send it off for display!
	XrFrameEndInfo end_info{ XR_TYPE_FRAME_END_INFO };
	end_info.displayTime          = frame_state.predictedDisplayTime;
	end_info.environmentBlendMode = xr_blend;
	end_info.layerCount           = layer == nullptr ? 0 : 1;
	end_info.layers               = &layer;
	xrEndFrame(xr_session, &end_info);

	render_clear();
}

///////////////////////////////////////////

void openxr_projection(XrFovf fov, float clip_near, float clip_far, float *result) {
	// Mix of XMMatrixPerspectiveFovRH from DirectXMath and XrMatrix4x4f_CreateProjectionFov from xr_linear.h
	const float tanLeft        = tanf(fov.angleLeft);
	const float tanRight       = tanf(fov.angleRight);
	const float tanDown        = tanf(fov.angleDown);
	const float tanUp          = tanf(fov.angleUp);
	const float tanAngleWidth  = tanRight - tanLeft;
	const float tanAngleHeight = tanUp - tanDown;
	const float range          = clip_far / (clip_near - clip_far);

	// [row][column]
	memset(result, 0, sizeof(float) * 16);
	result[0]  = 2 / tanAngleWidth;                    // [0][0] Different, DX uses: Width (Height / AspectRatio);
	result[5]  = 2 / tanAngleHeight;                   // [1][1] Same as DX's: Height (CosFov / SinFov)
	result[8]  = (tanRight + tanLeft) / tanAngleWidth; // [2][0] Only present in xr's
	result[9]  = (tanUp + tanDown) / tanAngleHeight;   // [2][1] Only present in xr's
	result[10] = range;                               // [2][2] Same as xr's: -(farZ + offsetZ) / (farZ - nearZ)
	result[11] = -1;                                  // [2][3] Same
	result[14] = range * clip_near;                   // [3][2] Same as xr's: -(farZ * (nearZ + offsetZ)) / (farZ - nearZ);
}

///////////////////////////////////////////

bool openxr_render_layer(XrTime predictedTime, vector<XrCompositionLayerProjectionView> &views, XrCompositionLayerProjection &layer) {

	// Find the state and location of each viewpoint at the predicted time
	uint32_t         view_count  = 0;
	XrViewState      view_state  = { XR_TYPE_VIEW_STATE };
	XrViewLocateInfo locate_info = { XR_TYPE_VIEW_LOCATE_INFO };
	locate_info.viewConfigurationType = app_config_view;
	locate_info.displayTime           = predictedTime;
	locate_info.space                 = xr_app_space;
	xrLocateViews(xr_session, &locate_info, &view_state, (uint32_t)xr_views.size(), &view_count, xr_views.data());
	views.resize(view_count);

	// And now we'll iterate through each viewpoint, and render it!
	for (uint32_t i = 0; i < view_count; i++) {

		// We need to ask which swapchain image to use for rendering! Which one will we get?
		// Who knows! It's up to the runtime to decide.
		uint32_t                    img_id;
		XrSwapchainImageAcquireInfo acquire_info = { XR_TYPE_SWAPCHAIN_IMAGE_ACQUIRE_INFO };
		xrAcquireSwapchainImage(xr_swapchains[i].handle, &acquire_info, &img_id);

		// Wait until the image is available to render to. The compositor could still be
		// reading from it.
		XrSwapchainImageWaitInfo wait_info = { XR_TYPE_SWAPCHAIN_IMAGE_WAIT_INFO };
		wait_info.timeout = XR_INFINITE_DURATION;
		xrWaitSwapchainImage(xr_swapchains[i].handle, &wait_info);

		// Set up our rendering information for the viewpoint we're using right now!
		views[i] = { XR_TYPE_COMPOSITION_LAYER_PROJECTION_VIEW };
		views[i].pose = xr_views[i].pose;
		views[i].fov  = xr_views[i].fov;
		views[i].subImage.swapchain        = xr_swapchains[i].handle;
		views[i].subImage.imageRect.offset = { 0, 0 };
		views[i].subImage.imageRect.extent = { xr_swapchains[i].width, xr_swapchains[i].height };

		// Call the rendering callback with our view and swapchain info
		tex2d_t target = xr_swapchains[i].surface_data[img_id];
		tex2d_rtarget_clear(target, sk_info.display_type == display_opaque ? color32{0,0,0,255} : color32{0, 0, 0, 0});
		tex2d_rtarget_set_active(target);
		D3D11_VIEWPORT viewport = CD3D11_VIEWPORT(
			(float)views[i].subImage.imageRect.offset.x, 
			(float)views[i].subImage.imageRect.offset.y, 
			(float)views[i].subImage.imageRect.extent.width, 
			(float)views[i].subImage.imageRect.extent.height);
		d3d_context->RSSetViewports(1, &viewport);

		float xr_projection[16];
		matrix xr_proj_m;
		openxr_projection(views[i].fov, 0.1f, 50, xr_projection);
		memcpy(&xr_proj_m, xr_projection, sizeof(float) * 16);
		transform_t cam_transform;
		transform_set(cam_transform, (vec3&)views[i].pose.position, vec3_one, (quat&)views[i].pose.orientation);
		render_draw_matrix(xr_proj_m, cam_transform);

		// And tell OpenXR we're done with rendering to this one!
		XrSwapchainImageReleaseInfo release_info = { XR_TYPE_SWAPCHAIN_IMAGE_RELEASE_INFO };
		xrReleaseSwapchainImage(xr_swapchains[i].handle, &release_info);
	}

	layer.space     = xr_app_space;
	layer.viewCount = (uint32_t)views.size();
	layer.views     = views.data();
	return true;
}

///////////////////////////////////////////

void openxr_make_actions() {
	XrActionSetCreateInfo actionset_info = { XR_TYPE_ACTION_SET_CREATE_INFO };
	strcpy_s(actionset_info.actionSetName,          "input");
	strcpy_s(actionset_info.localizedActionSetName, "Input");
	xrCreateActionSet(xr_instance, &actionset_info, &xr_input.action_set);
	xrStringToPath(xr_instance, "/user/hand/left",  &xr_input.handSubactionPath[0]);
	xrStringToPath(xr_instance, "/user/hand/right", &xr_input.handSubactionPath[1]);

	// Create an action to track the position and orientation of the hands! This is
	// the controller location, or the center of the palms for actual hands.
	XrActionCreateInfo action_info = { XR_TYPE_ACTION_CREATE_INFO };
	action_info.countSubactionPaths = _countof(xr_input.handSubactionPath);
	action_info.subactionPaths      = xr_input.handSubactionPath;
	action_info.actionType          = XR_ACTION_TYPE_POSE_INPUT;
	strcpy_s(action_info.actionName,          "hand_pose");
	strcpy_s(action_info.localizedActionName, "Hand Pose");
	xrCreateAction(xr_input.action_set, &action_info, &xr_input.poseAction);

	// Create an action for listening to the select action! This is primary trigger
	// on controllers, and an airtap on HoloLens
	action_info.actionType = XR_ACTION_TYPE_BOOLEAN_INPUT;
	strcpy_s(action_info.actionName,          "select");
	strcpy_s(action_info.localizedActionName, "Select");
	xrCreateAction(xr_input.action_set, &action_info, &xr_input.selectAction);

	action_info.actionType = XR_ACTION_TYPE_BOOLEAN_INPUT;
	strcpy_s(action_info.actionName,          "grip");
	strcpy_s(action_info.localizedActionName, "Grip");
	xrCreateAction(xr_input.action_set, &action_info, &xr_input.gripAction);

	// Bind the actions we just created to specific locations on the Khronos simple_controller
	// definition! These are labeled as 'suggested' because they may be overridden by the runtime
	// preferences. For example, if the runtime allows you to remap buttons, or provides input
	// accessibility settings.
	XrPath profile_path;
	XrPath pose_path  [2];
	XrPath select_path[2];
	XrPath grip_path  [2];
	xrStringToPath(xr_instance, "/user/hand/left/input/aim/pose",       &pose_path[0]);
	xrStringToPath(xr_instance, "/user/hand/right/input/aim/pose",      &pose_path[1]);
	XrInteractionProfileSuggestedBinding suggested_binds = { XR_TYPE_INTERACTION_PROFILE_SUGGESTED_BINDING };

	// microsoft / motion_controller
	{
		xrStringToPath(xr_instance, "/user/hand/left/input/trigger/value",  &select_path[0]);
		xrStringToPath(xr_instance, "/user/hand/right/input/trigger/value", &select_path[1]);
		xrStringToPath(xr_instance, "/user/hand/left/input/squeeze/click",  &grip_path[0]);
		xrStringToPath(xr_instance, "/user/hand/right/input/squeeze/click", &grip_path[1]);
		XrActionSuggestedBinding bindings[] = {
			{ xr_input.poseAction,   pose_path  [0] }, { xr_input.poseAction,   pose_path  [1] },
			{ xr_input.selectAction, select_path[0] }, { xr_input.selectAction, select_path[1] },
			{ xr_input.gripAction,   grip_path  [0] }, { xr_input.gripAction,   grip_path  [1] }
		};

		xrStringToPath(xr_instance, "/interaction_profiles/microsoft/motion_controller", &profile_path);
		suggested_binds.interactionProfile     = profile_path;
		suggested_binds.suggestedBindings      = &bindings[0];
		suggested_binds.countSuggestedBindings = _countof(bindings);
		xrSuggestInteractionProfileBindings(xr_instance, &suggested_binds);
	}

	// khr / simple_controller
	{
		xrStringToPath(xr_instance, "/user/hand/left/input/select/click",  &select_path[0]);
		xrStringToPath(xr_instance, "/user/hand/right/input/select/click", &select_path[1]);
		XrActionSuggestedBinding bindings[] = {
			{ xr_input.poseAction,   pose_path  [0] }, { xr_input.poseAction,   pose_path  [1] },
			{ xr_input.selectAction, select_path[0] }, { xr_input.selectAction, select_path[1] },
		};

		xrStringToPath(xr_instance, "/interaction_profiles/khr/simple_controller", &profile_path);
		suggested_binds.interactionProfile     = profile_path;
		suggested_binds.suggestedBindings      = &bindings[0];
		suggested_binds.countSuggestedBindings = _countof(bindings);
		xrSuggestInteractionProfileBindings(xr_instance, &suggested_binds);
	}

	// Create frames of reference for the pose actions
	for (int32_t i = 0; i < 2; i++) {
		XrActionSpaceCreateInfo action_space_info = { XR_TYPE_ACTION_SPACE_CREATE_INFO };
		action_space_info.poseInActionSpace = { {0,0,0,1}, {0,0,0} };
		action_space_info.subactionPath     = xr_input.handSubactionPath[i];
		action_space_info.action            = xr_input.poseAction;
		xrCreateActionSpace(xr_session, &action_space_info, &xr_input.handSpace[i]);
	}

	// Attach the action set we just made to the session
	XrSessionActionSetsAttachInfo attach_info = { XR_TYPE_SESSION_ACTION_SETS_ATTACH_INFO };
	attach_info.countActionSets = 1;
	attach_info.actionSets      = &xr_input.action_set;
	xrAttachSessionActionSets(xr_session, &attach_info);

	for (int32_t i = 1; i >= 0; i--) {
		input_source_ hand = i == 0 ? input_source_hand_left : input_source_hand_right;
		xr_input.pointer_ids[i] = input_add_pointer(input_source_can_press | input_source_hand | hand);
	}
	xr_input.pointer_ids[2] = input_add_pointer(input_source_gaze | input_source_gaze_head);
}

///////////////////////////////////////////

void openxr_poll_actions() {
	if (xr_session_state != XR_SESSION_STATE_FOCUSED)
		return;

	// Update our action set with up-to-date input data!
	XrActiveActionSet action_set = { };
	action_set.actionSet     = xr_input.action_set;
	action_set.subactionPath = XR_NULL_PATH;
	XrActionsSyncInfo sync_info = { XR_TYPE_ACTIONS_SYNC_INFO };
	sync_info.countActiveActionSets = 1;
	sync_info.activeActionSets      = &action_set;
	xrSyncActions(xr_session, &sync_info);

	// Now we'll get the current states of our actions, and store them for later use
	for (uint32_t hand = 0; hand < handed_max; hand++) {
		XrActionStateGetInfo get_info = { XR_TYPE_ACTION_STATE_GET_INFO };
		get_info.subactionPath = xr_input.handSubactionPath[hand];

		XrActionStatePose pose_state = { XR_TYPE_ACTION_STATE_POSE };
		get_info.action = xr_input.poseAction;
		xrGetActionStatePose(xr_session, &get_info, &pose_state);
		xr_input.renderHand[hand] = pose_state.isActive;

		// Events come with a timestamp
		XrActionStateBoolean select_state = { XR_TYPE_ACTION_STATE_BOOLEAN };
		get_info.action = xr_input.selectAction;
		xrGetActionStateBoolean(xr_session, &get_info, &select_state);
		xr_input.handSelect[hand] = select_state.currentState && select_state.changedSinceLastSync;

		// Events come with a timestamp
		XrActionStateBoolean grip_state = { XR_TYPE_ACTION_STATE_BOOLEAN };
		get_info.action = xr_input.gripAction;
		xrGetActionStateBoolean(xr_session, &get_info,  &grip_state);

		// If we have a select event, update the hand pose to match the event's timestamp
		XrSpaceLocation space_location = { XR_TYPE_SPACE_LOCATION };
		XrResult        res            = xrLocateSpace(xr_input.handSpace[hand], xr_app_space, xr_time, &space_location);
		if (XR_UNQUALIFIED_SUCCESS(res) && openxr_loc_valid(space_location)) {
			xr_input.handPose[hand] = space_location.pose;
		}

		pointer_t *pointer = input_get_pointer(xr_input.pointer_ids[hand]);
		pointer->state = pose_state.isActive ? pointer_state_available : pointer_state_none;
		openxr_pose_to_pointer(space_location.pose, pointer);

		input_hand_sim((handed_)hand, pointer->ray.pos, pointer->orientation, pose_state.isActive, select_state.currentState, grip_state.currentState );

		// Get event poses, and fire our own events for them
		const hand_t &curr_hand = input_hand((handed_)hand);
		if ((curr_hand.state & input_state_justpinch) || (curr_hand.state & input_state_unpinch)) {
			space_location = { XR_TYPE_SPACE_LOCATION };
			res            = xrLocateSpace(xr_input.handSpace[hand], xr_app_space, select_state.lastChangeTime, &space_location);
			if (XR_UNQUALIFIED_SUCCESS(res) && openxr_loc_valid(space_location)) {
				pointer_t event_pointer = *pointer;
				openxr_pose_to_pointer(space_location.pose, &event_pointer);

				input_fire_event(
					input_source_hand | (hand == handed_left ? input_source_hand_left :input_source_hand_right), 
					curr_hand.state & input_state_pinch ? input_state_justpinch : input_state_unpinch, 
					event_pointer);
			}
		}
		if ((curr_hand.state & input_state_justgrip) || (curr_hand.state & input_state_ungrip)) {
			space_location = { XR_TYPE_SPACE_LOCATION };
			res            = xrLocateSpace(xr_input.handSpace[hand], xr_app_space, grip_state.lastChangeTime, &space_location);
			if (XR_UNQUALIFIED_SUCCESS(res) && openxr_loc_valid(space_location)) {
				pointer_t event_pointer = *pointer;
				openxr_pose_to_pointer(space_location.pose, &event_pointer);

				input_fire_event(
					input_source_hand | (hand == handed_left ? input_source_hand_left :input_source_hand_right),
					curr_hand.state & input_state_grip ? input_state_justgrip : input_state_ungrip, 
					event_pointer);
			}
		}
		if (curr_hand.state & input_state_justtracked) {
			input_fire_event(
				input_source_hand | (hand == handed_left ? input_source_hand_left :input_source_hand_right),
				input_state_justtracked, 
				*pointer);
		}
		if (curr_hand.state & input_state_untracked) {
			input_fire_event(
				input_source_hand | (hand == handed_left ? input_source_hand_left :input_source_hand_right),
				input_state_untracked, 
				*pointer);
		}
	}
}

///////////////////////////////////////////

void openxr_pose_to_pointer(XrPosef &pose, pointer_t *pointer) {
	memcpy(&pointer->ray.pos,     &pose.position,    sizeof(vec3));
	memcpy(&pointer->orientation, &pose.orientation, sizeof(quat));
	pointer->ray.dir = pointer->orientation * vec3_forward;
}

} // namespace sk