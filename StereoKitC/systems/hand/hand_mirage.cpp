#include "../../stereokit.h"
#include "../platform/openxr.h"
#include "../platform/openxr_input.h"
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
SpatialStationaryFrameOfReference mirage_spatial_stage       = nullptr;
SpatialInteractionManager         mirage_interaction_manager = nullptr;
bool                              mirage_hand_support        = false;
hand_joint_t                      mirage_hand_data[2][27]    = {};
bool                              mirage_checked             = false;
#endif

///////////////////////////////////////////

bool hand_mirage_available() {
#if WINDOWS_UWP
	if (mirage_checked)
		return mirage_hand_support;
	mirage_checked = true;

	if (sk_active_runtime() != runtime_mixedreality) {
		xr_hand_state       = xr_hand_state_unavailable;
		mirage_hand_support = false;
		return false;
	}

	CoreApplication::MainView().CoreWindow().Dispatcher().RunAsync(CoreDispatcherPriority::Normal, []() {
		mirage_spatial_stage       = SpatialLocator::GetDefault().CreateStationaryFrameOfReferenceAtCurrentLocation();
		mirage_interaction_manager = SpatialInteractionManager::GetForCurrentView();
		mirage_hand_support        = mirage_interaction_manager.IsSourceKindSupported(SpatialInteractionSourceKind::Hand);

		xr_hand_state = mirage_hand_support == true
			? xr_hand_state_present 
			: xr_hand_state_unavailable;

		input_hand_refresh_system();
	});

	return mirage_hand_support;
#else
	xr_hand_state = xr_hand_state_unavailable;
	return false;
#endif
}

///////////////////////////////////////////

void hand_mirage_init(){
#if !WINDOWS_UWP
	log_err("Mirage hands aren't available in this build!");
#endif
}

///////////////////////////////////////////

void hand_mirage_shutdown() {
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
	if (!mirage_hand_support)
		return;

	oxri_update_frame();

	// Convert the time we're given into a format that Windows likes
	PerceptionTimestamp stamp = PerceptionTimestampHelper::FromSystemRelativeTargetTime(TimeSpanFromQpcTicks(win32_prediction_time));
	IVectorView<SpatialInteractionSourceState> sources = mirage_interaction_manager.GetDetectedSourcesAtTimestamp(stamp);

	for (auto sourceState : sources)
	{
		HandPose pose = sourceState.TryGetHandPose();
		if (!pose || !mirage_spatial_stage)
			continue;

		// Grab the joints from windows
		JointPose               poses[27];
		SpatialCoordinateSystem coordinates = mirage_spatial_stage.CoordinateSystem();
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
			openxr_get_space(xr_hand_space[handed], hand_to_world);
			
			// Aaaand convert!
			for (size_t i = 0; i < 27; i++) {
				memcpy(&mirage_hand_data[handed][i].position,    &poses[i].Position,    sizeof(vec3));
				memcpy(&mirage_hand_data[handed][i].orientation, &poses[i].Orientation, sizeof(quat));
				mirage_hand_data[handed][i].position    = hand_to_origin.orientation * (mirage_hand_data[handed][i].position - hand_to_origin.position);
				mirage_hand_data[handed][i].position    = (hand_to_world.orientation * mirage_hand_data[handed][i].position) + hand_to_world.position; 
				mirage_hand_data[handed][i].orientation = mirage_hand_data[handed][i].orientation * hand_to_origin.orientation;
				mirage_hand_data[handed][i].orientation = mirage_hand_data[handed][i].orientation * hand_to_world.orientation;
				mirage_hand_data[handed][i].radius      = poses[i].Radius * 1.2f;
			}
			
			// Copy the data into the input system
			hand_t& inp_hand = (hand_t&)input_hand(handed);
			inp_hand.tracked_state = button_state_active;
			hand_joint_t* hand_poses = input_hand_get_pose_buffer(handed);
			memcpy(hand_poses, &mirage_hand_data[handed][0], sizeof(hand_joint_t) * 25);

			quat face_forward = quat_from_angles(-90,0,0);
			inp_hand.palm  = pose_t{ mirage_hand_data[handed][25].position, face_forward * mirage_hand_data[handed][25].orientation };
			inp_hand.wrist = pose_t{ mirage_hand_data[handed][26].position, face_forward * quat_from_angles(-90,0,0) * mirage_hand_data[handed][26].orientation };
		} else {
			log_warn("Couldn't get hand joints!");
		}
	}
#endif
}

///////////////////////////////////////////

void hand_mirage_update_frame() {
	if (xr_time == 0) return;

	hand_mirage_update_hands(openxr_get_time());
}

///////////////////////////////////////////

void hand_mirage_update_predicted() {
	if (xr_time == 0) return;

	hand_mirage_update_hands(openxr_get_time());
	input_hand_update_meshes();
}

}
