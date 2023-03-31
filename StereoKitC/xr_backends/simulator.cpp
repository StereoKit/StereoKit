#include "simulator.h"

#include "../device.h"
#include "../libraries/stref.h"
#include "../platforms/flatscreen_input.h"
#include "../systems/input.h"

namespace sk {

///////////////////////////////////////////

bool simulator_init() {
	device_data.has_hand_tracking = true;
	device_data.has_eye_gaze      = true;
	device_data.tracking          = device_tracking_6dof;
	device_data.name              = string_copy("Simulator");

	flatscreen_input_init();
	return true;
}

///////////////////////////////////////////

void simulator_shutdown() {
	flatscreen_input_shutdown();
}

///////////////////////////////////////////

void simulator_step_begin() {
	flatscreen_input_update();
}

///////////////////////////////////////////

void simulator_step_end() {
	input_update_poses(true);
}

///////////////////////////////////////////

void simulator_set_origin_offset(pose_t offset) {
}


}