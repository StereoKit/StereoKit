#include "simulator.h"

#include "../device.h"
#include "../libraries/stref.h"

namespace sk {

///////////////////////////////////////////

bool none_init() {
	device_data.has_hand_tracking = false;
	device_data.has_eye_gaze      = false;
	device_data.tracking          = device_tracking_none;
	device_data.name              = string_copy("None");
	return true;
}

///////////////////////////////////////////

void none_shutdown() {
}

///////////////////////////////////////////

void none_step_begin() {
}

///////////////////////////////////////////

void none_step_end() {
}

}