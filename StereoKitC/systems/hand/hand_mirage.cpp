#include "../../stereokit.h"
#include "../platform/openxr.h"
#include "input_hand.h"

#if WINDOWS_UWP
#include <chrono>

#include "winrt/Windows.UI.Core.h" 
#include "winrt/Windows.UI.Input.Spatial.h" 
#include "winrt/Windows.Perception.h"
#include "winrt/Windows.Perception.People.h"
#include "winrt/Windows.Perception.Spatial.h"
#include "winrt/Windows.Foundation.Numerics.h"
#include "winrt/Windows.ApplicationModel.Core.h"
#include "winrt/Windows.Foundation.Collections.h"
#include "winrt/Windows.Foundation.h"

using namespace winrt::Windows::UI::Core;
using namespace winrt::Windows::UI::Input::Spatial;
using namespace winrt::Windows::Perception;
using namespace winrt::Windows::Perception::People;
using namespace winrt::Windows::Perception::Spatial;
using namespace winrt::Windows::Foundation::Numerics;
using namespace winrt::Windows::ApplicationModel::Core;
using namespace winrt::Windows::Foundation::Collections;
using namespace winrt::Windows::Foundation;
#endif

namespace sk {

#if WINDOWS_UWP
SpatialStationaryFrameOfReference xr_spatial_stage       = nullptr;
SpatialInteractionManager         xr_interaction_manager = nullptr;
bool                              xr_hand_support        = false;
hand_joint_t xr_hand_data[2][27] = {};
bool    mirage_checked = false;

XrActionSet mirage_action_set;
XrAction    mirage_pose_action;
XrPath      mirage_hand_subaction_path[2];
XrSpace     mirage_hand_space[2] = {};
#endif



///////////////////////////////////////////

bool hand_mirage_available() {
#if WINDOWS_UWP
	if (mirage_checked)
		return xr_hand_support;
	mirage_checked = true;

	CoreApplication::MainView().CoreWindow().Dispatcher().RunAsync(CoreDispatcherPriority::Normal, []() {
		xr_spatial_stage       = SpatialLocator::GetDefault().CreateStationaryFrameOfReferenceAtCurrentLocation();
		xr_interaction_manager = SpatialInteractionManager::GetForCurrentView();
		xr_hand_support        = xr_interaction_manager.IsSourceKindSupported(SpatialInteractionSourceKind::Hand);

		xr_hand_state = xr_hand_support == true
			? xr_hand_state_present 
			: xr_hand_state_unavailable;

		input_hand_refresh_system();
	});

	return xr_hand_support;
#else
	xr_hand_state = xr_hand_state_unavailable;
	return false;
#endif
}

///////////////////////////////////////////

void hand_mirage_init(){
#if WINDOWS_UWP
	XrActionSetCreateInfo actionset_info = { XR_TYPE_ACTION_SET_CREATE_INFO };
	strcpy_s(actionset_info.actionSetName,          "mirage_input");
	strcpy_s(actionset_info.localizedActionSetName, "MirageInput");
	XrResult result = xrCreateActionSet(xr_instance, &actionset_info, &mirage_action_set);
	xrStringToPath(xr_instance, "/user/hand/left",  &mirage_hand_subaction_path[0]);
	xrStringToPath(xr_instance, "/user/hand/right", &mirage_hand_subaction_path[1]);
	if (XR_FAILED(result)) {
		log_infof("xrCreateActionSet failed: [%s]", openxr_string(result));
		return;
	}

	// Create an action to track the position and orientation of the hands! This is
	// the controller location, or the center of the palms for actual hands.
	XrActionCreateInfo action_info = { XR_TYPE_ACTION_CREATE_INFO };
	action_info.countSubactionPaths = _countof(mirage_hand_subaction_path);
	action_info.subactionPaths      = mirage_hand_subaction_path;
	action_info.actionType          = XR_ACTION_TYPE_POSE_INPUT;
	strcpy_s(action_info.actionName,          "mirage_hand_pose");
	strcpy_s(action_info.localizedActionName, "Mirage Hand Pose");
	result = xrCreateAction(mirage_action_set, &action_info, &mirage_pose_action);
	if (XR_FAILED(result)) {
		log_infof("xrCreateAction failed: [%s]", openxr_string(result));
		return;
	}

	// Bind the actions we just created to specific locations on the Khronos simple_controller
	// definition! These are labeled as 'suggested' because they may be overridden by the runtime
	// preferences. For example, if the runtime allows you to remap buttons, or provides input
	// accessibility settings.
	XrPath profile_path;
	XrPath pose_path  [2];
	xrStringToPath(xr_instance, "/user/hand/left/input/grip/pose",  &pose_path[0]);
	xrStringToPath(xr_instance, "/user/hand/right/input/grip/pose", &pose_path[1]);
	XrInteractionProfileSuggestedBinding suggested_binds = { XR_TYPE_INTERACTION_PROFILE_SUGGESTED_BINDING };

	// microsoft / motion_controller
	{
		XrActionSuggestedBinding bindings[] = {
			{ mirage_pose_action, pose_path[0] }, { mirage_pose_action, pose_path[1] },
		};

		xrStringToPath(xr_instance, "/interaction_profiles/microsoft/motion_controller", &profile_path);
		suggested_binds.interactionProfile     = profile_path;
		suggested_binds.suggestedBindings      = &bindings[0];
		suggested_binds.countSuggestedBindings = _countof(bindings);
		result = xrSuggestInteractionProfileBindings(xr_instance, &suggested_binds);
		if (XR_FAILED(result)) {
			log_infof("xrSuggestInteractionProfileBindings failed: [%s]", openxr_string(result));
			return;
		}
	}

	// khr / simple_controller
	{
		XrActionSuggestedBinding bindings[] = {
			{ mirage_pose_action, pose_path[0] }, { mirage_pose_action, pose_path[1] },
		};

		xrStringToPath(xr_instance, "/interaction_profiles/khr/simple_controller", &profile_path);
		suggested_binds.interactionProfile     = profile_path;
		suggested_binds.suggestedBindings      = &bindings[0];
		suggested_binds.countSuggestedBindings = _countof(bindings);
		result = xrSuggestInteractionProfileBindings(xr_instance, &suggested_binds);
		if (XR_FAILED(result)) {
			log_infof("xrSuggestInteractionProfileBindings failed: [%s]", openxr_string(result));
			return;
		}
	}

	// Create frames of reference for the pose actions
	for (int32_t i = 0; i < 2; i++) {
		XrActionSpaceCreateInfo action_space_info = { XR_TYPE_ACTION_SPACE_CREATE_INFO };
		action_space_info.poseInActionSpace = { {0,0,0,1}, {0,0,0} };
		action_space_info.subactionPath     = mirage_hand_subaction_path[i];
		action_space_info.action            = mirage_pose_action;
		result = xrCreateActionSpace(xr_session, &action_space_info, &mirage_hand_space[i]);
		if (XR_FAILED(result)) {
			log_infof("xrCreateActionSpace failed: [%s]", openxr_string(result));
			return;
		}
	}

	// Attach the action set we just made to the session
	XrSessionActionSetsAttachInfo attach_info = { XR_TYPE_SESSION_ACTION_SETS_ATTACH_INFO };
	attach_info.countActionSets = 1;
	attach_info.actionSets      = &mirage_action_set;
	result = xrAttachSessionActionSets(xr_session, &attach_info);
	if (XR_FAILED(result)) {
		log_infof("xrAttachSessionActionSets failed: [%s]", openxr_string(result));
		return;
	}
#endif

#if !WINDOWS_UWP
	log_err("Mirage hands aren't available in this build!");
#endif
}

///////////////////////////////////////////

void hand_mirage_shutdown() {
#if WINDOWS_UWP
	xrDestroySpace(mirage_hand_space[0]);
	xrDestroySpace(mirage_hand_space[1]);
	xrDestroyActionSet(mirage_action_set);
#endif
}

///////////////////////////////////////////

#if WINDOWS_UWP
// Time conversion code from David Fields, thank you!
static inline int64_t SourceDurationTicksToDestDurationTicks(int64_t sourceDurationInTicks, int64_t sourceTicksPerSecond, int64_t destTicksPerSecond) {
    int64_t whole = (sourceDurationInTicks / sourceTicksPerSecond) * destTicksPerSecond;                          // 'whole' is rounded down in the target time units.
    int64_t part  = (sourceDurationInTicks % sourceTicksPerSecond) * destTicksPerSecond / sourceTicksPerSecond;   // 'part' is the remainder in the target time units.
    return whole + part;
}
static inline TimeSpan TimeSpanFromQpcTicks(int64_t qpcTicks) {
    static const int64_t qpcFrequency = [](){
        LARGE_INTEGER frequency;
        QueryPerformanceFrequency(&frequency);
        return frequency.QuadPart;
    }();
    return TimeSpan{ SourceDurationTicksToDestDurationTicks(qpcTicks, qpcFrequency, winrt::clock::period::den) / winrt::clock::period::num };
}
#endif

///////////////////////////////////////////

void hand_mirage_update_hands(int64_t win32_prediction_time) {
#if WINDOWS_UWP
	if (!xr_hand_support)
		return;

	// Update our action set with up-to-date input data!
	XrActiveActionSet action_set = { };
	action_set.actionSet     = mirage_action_set;
	action_set.subactionPath = XR_NULL_PATH;
	XrActionsSyncInfo sync_info = { XR_TYPE_ACTIONS_SYNC_INFO };
	sync_info.countActiveActionSets = 1;
	sync_info.activeActionSets      = &action_set;
	xrSyncActions(xr_session, &sync_info);

	// Convert the time we're given into a format that Windows likes
	PerceptionTimestamp stamp = PerceptionTimestampHelper::FromSystemRelativeTargetTime(TimeSpanFromQpcTicks(win32_prediction_time));
	IVectorView<SpatialInteractionSourceState> sources = xr_interaction_manager.GetDetectedSourcesAtTimestamp(stamp);

	for (auto sourceState : sources)
	{
		HandPose pose = sourceState.TryGetHandPose();
		if (!pose || !xr_spatial_stage)
			continue;

		// Grab the joints from windows
		JointPose               poses[27];
		SpatialCoordinateSystem coordinates = xr_spatial_stage.CoordinateSystem();
		handed_                 handed      = sourceState.Source().Handedness() == SpatialInteractionSourceHandedness::Left ? handed_left : handed_right;
		bool gotJoints = pose.TryGetJoints(
			coordinates,
			{   HandJointKind::ThumbMetacarpal,  HandJointKind::ThumbMetacarpal,  HandJointKind::ThumbProximal,      HandJointKind::ThumbDistal,  HandJointKind::ThumbTip,
				HandJointKind::IndexMetacarpal,  HandJointKind::IndexProximal,    HandJointKind::IndexIntermediate,  HandJointKind::IndexDistal,  HandJointKind::IndexTip,
				HandJointKind::MiddleMetacarpal, HandJointKind::MiddleProximal,   HandJointKind::MiddleIntermediate, HandJointKind::MiddleDistal, HandJointKind::MiddleTip,
				HandJointKind::RingMetacarpal,   HandJointKind::RingProximal,     HandJointKind::RingIntermediate,   HandJointKind::RingDistal,   HandJointKind::RingTip,
				HandJointKind::LittleMetacarpal, HandJointKind::LittleProximal,   HandJointKind::LittleIntermediate, HandJointKind::LittleDistal, HandJointKind::LittleTip,
				HandJointKind::Palm, HandJointKind::Wrist, },
			poses);
		
		// Convert the data from their format to ours
		if (gotJoints) {
			SpatialInteractionSourceLocation location = sourceState.Properties().TryGetLocation(coordinates);

			// Take it from Mirage coordinates to the origin
			pose_t hand_to_origin;
			memcpy(&hand_to_origin.position,    &location.Position().Value(),    sizeof(vec3));
			memcpy(&hand_to_origin.orientation, &location.Orientation().Value(), sizeof(quat));
			hand_to_origin.orientation = quat_inverse(hand_to_origin.orientation);

			// Take it from the origin, to our coordinates
			pose_t hand_to_world = {};
			openxr_get_space(mirage_hand_space[handed], hand_to_world);
			
			// Aaaand convert!
			for (size_t i = 0; i < 27; i++) {
				memcpy(&xr_hand_data[handed][i].position,    &poses[i].Position,    sizeof(vec3));
				memcpy(&xr_hand_data[handed][i].orientation, &poses[i].Orientation, sizeof(quat));
				xr_hand_data[handed][i].position    = hand_to_origin.orientation * (xr_hand_data[handed][i].position - hand_to_origin.position);
				xr_hand_data[handed][i].position    = (hand_to_world.orientation * xr_hand_data[handed][i].position) + hand_to_world.position; 
				xr_hand_data[handed][i].orientation = xr_hand_data[handed][i].orientation * hand_to_origin.orientation;
				xr_hand_data[handed][i].orientation = xr_hand_data[handed][i].orientation * hand_to_world.orientation;
				xr_hand_data[handed][i].radius = (poses[i].Radius * 0.85f);
			}
			
			// Copy the data into the input system
			hand_t& inp_hand = (hand_t&)input_hand(handed);
			inp_hand.tracked_state = button_state_active;
			hand_joint_t* hand_poses = input_hand_get_pose_buffer(handed);
			memcpy(hand_poses, &xr_hand_data[handed][0], sizeof(hand_joint_t) * 25);

			quat face_forward = quat_from_angles(-90,0,0);
			inp_hand.palm  = pose_t{ xr_hand_data[handed][25].position, face_forward * xr_hand_data[handed][25].orientation };
			inp_hand.wrist = pose_t{ xr_hand_data[handed][26].position, face_forward * quat_from_angles(-90,0,0) * xr_hand_data[handed][26].orientation };
		} else {
			log_warn("Couldn't get hand joints!");
		}
	}
#endif
}

///////////////////////////////////////////

void hand_mirage_update_frame() {
	hand_mirage_update_hands(openxr_get_time());
}

///////////////////////////////////////////

void hand_mirage_update_predicted() {
	hand_mirage_update_hands(openxr_get_time());
	input_hand_update_meshes();
}

}